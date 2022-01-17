#ifndef IFCA_H
#define IFCA_H

#include <type_traits>

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "ifca/ifcaMethods.hpp"
#include "ifca/isIfcaInterface.hpp"
#include "ifca/maxParallel.hpp"
#include "ifca/transform/isTransformExpression.hpp"
#include "ifca/transform/transformExpression.hpp"
#include "transformChain.hpp"
#include "types.hpp"

// TODO: Lock methods returning void type if not final
namespace ifca {

/**
 * @brief Base Ifca template
 *
 * @tparam In Begining of transforms of Ifca
 * @tparam Out Last transform in Ifca
 * @tparam IsTransformChain Template flag to help deduce template type
 */
template <typename In, typename Out, typename IsTransformChain = void>
struct IfcaImpl;

/**
 * @brief Empty ifca
 *
 * @tparam InputType type of chunks passed to ifca
 */
template <typename InputType>
class IfcaImpl<InputType, InputType,
               std::enable_if_t<!is_transform_expression_v<InputType> &&
                                !is_ifca_interface_v<InputType> &&
                                !std::is_void_v<InputType>>>
    : public IfcaMethods<IfcaImpl<InputType, InputType>, InputType, InputType> {
 public:
  using Impl = IfcaImpl<InputType, InputType>;
  using input_type = InputType;
  using output_type = InputType;
  using base_type = IfcaMethods<Impl, input_type, output_type>;
  using transforms_type = transform_chain_t<>;

  explicit IfcaImpl(unsigned int max_parallel = maxParallel())
      : base_type(max_parallel){};

  template <typename Chunk>
  void operator()(Chunk&& chunk,
                  std::future<void>&& previous_processing_future) {
    this->onResolve(FWD(chunk), std::move(previous_processing_future));
  }

  template <typename Transform>
  typename std::enable_if_t<is_transform_expression_v<Transform> &&
                                !std::is_lvalue_reference_v<Transform>,
                            IfcaImpl<Transform, void>>
  operator+(Transform&& transform) {
    return IfcaImpl<Transform, void>(FWD(*this), FWD(transform));
  }

  template <typename, typename, typename>
  friend class IfcaImpl;
};

/**
 * @brief Helper function to create IfcaImpl
 *
 * @tparam Input
 * @param max_parallel
 * @return IfcaImpl<Input, Input, void>
 */
template <typename Input>
inline IfcaImpl<Input, Input, void> Ifca(
    unsigned int max_parallel = maxParallel()) {
  return IfcaImpl<Input, Input, void>(max_parallel);
}

/**
 * @brief Ifca with single transform
 *
 * @tparam FirstTransform
 */
template <typename FirstTransform>
class IfcaImpl<FirstTransform, void,
               std::enable_if_t<is_transform_expression_v<FirstTransform>>>
    : public IfcaMethods<IfcaImpl<FirstTransform, void>,
                         typename FirstTransform::input_type,
                         typename FirstTransform::output_type> {
 public:
  using Impl = IfcaImpl<FirstTransform, void>;
  using input_type = typename FirstTransform::input_type;
  using output_type = typename FirstTransform::output_type;
  using base_type = IfcaMethods<Impl, input_type, output_type>;
  using transforms_type = transform_chain_t<FirstTransform>;

  explicit IfcaImpl(FirstTransform&& firstTransform,
                    unsigned int max_parallel = maxParallel())
      : base_type(max_parallel),
        transforms_(transforms_type{std::move(firstTransform)}) {}

  template <typename CurrentIfca>
  explicit IfcaImpl(CurrentIfca&& currentIfca, FirstTransform&& firstTransform)
      : base_type(std::move(currentIfca)),
        transforms_(transforms_type{std::move(firstTransform)}) {}

  template <typename Chunk>
  void operator()(Chunk&& chunk,
                  std::future<void>&& previous_processing_future) {
    auto&& onResolveCallback = [&previous_processing_future,
                                this](output_type resolvedValue) -> void {
      this->onResolve(FWD(resolvedValue),
                      std::move(previous_processing_future));
    };

    auto&& onRejectCallback = [this] { this->onReject(); };
    std::get<0>(transforms_)(FWD(chunk), FWD(onResolveCallback),
                             FWD(onRejectCallback));
  }

  template <typename, typename, typename>
  friend class IfcaImpl;

 private:
  transforms_type transforms_;
};

/**
 * @brief Helper function to create IfcaImpl
 *
 * @tparam Transform
 * @param transform
 * @param max_parallel
 * @return IfcaImpl<Transform, void, void>>
 */
template <typename Transform>
inline typename std::enable_if_t<!std::is_lvalue_reference_v<Transform>,
                                 IfcaImpl<Transform, void, void>>
Ifca(Transform&& transform, unsigned int max_parallel = maxParallel()) {
  return IfcaImpl<Transform, void, void>(std::move(transform), max_parallel);
}

/**
 * @brief Ifca for transform chains
 *
 * @tparam CurrentIfca Ifca with transforms
 * @tparam NextTransform Transform to attach at end of transforms chain
 */
template <typename CurrentIfca, typename NextTransform>
class IfcaImpl<CurrentIfca, NextTransform,
               std::enable_if_t<is_ifca_interface_v<CurrentIfca> &&
                                is_transform_expression_v<NextTransform>>>
    : public IfcaMethods<IfcaImpl<CurrentIfca, NextTransform>,
                         typename CurrentIfca::input_type,
                         typename NextTransform::output_type> {
 public:
  using Impl = IfcaImpl<CurrentIfca, NextTransform>;
  using input_type = typename CurrentIfca::input_type;
  using output_type = typename NextTransform::output_type;
  using base_type = IfcaMethods<Impl, input_type, output_type>;
  using transforms_type =
      transform_chain_t<NextTransform, typename CurrentIfca::transforms_type>;

  explicit IfcaImpl(CurrentIfca&& currentIfca, NextTransform&& nextTransform)
      : base_type(std::move(currentIfca)),
        transforms_(ForwardTransformChain<typename CurrentIfca::transforms_type,
                                          NextTransform>(
            std::move(currentIfca.transforms_), std::move(nextTransform))){};

  template <typename Chunk>
  void operator()(Chunk&& chunk,
                  std::future<void>&& previous_processing_future) {
    runTransforms(
        FWD(chunk), std::move(previous_processing_future),
        std::make_index_sequence<std::tuple_size<transforms_type>{}>{});
  }

 protected:
  template <typename Chunk, std::size_t... Is>
  void runTransforms(Chunk&& chunk,
                     std::future<void>&& previous_processing_future,
                     std::index_sequence<Is...>) {
    runTransformsImpl(FWD(chunk), std::move(previous_processing_future),
                      std::get<Is>(transforms_)...);
  }

  template <typename Chunk, typename FirstTransform,
            typename... RestOfTransforms>
  void runTransformsImpl(Chunk&& chunk,
                         std::future<void>&& previous_processing_future,
                         FirstTransform&& firstTransform,
                         RestOfTransforms&&... restOfTransforms) {
    auto&& onResolveCallback =
        [&previous_processing_future, this](
            decltype(std::forward<output_type>(chunk)) resolvedValue) -> void {
      this->onResolve(FWD(resolvedValue),
                      std::move(previous_processing_future));
    };
    auto&& onRejectCallback = [this] { this->onReject(); };
    firstTransform(FWD(chunk), FWD(onResolveCallback), FWD(onRejectCallback),
                   FWD(restOfTransforms)...);
  }

  template <typename, typename, typename>
  friend class IfcaImpl;

 private:
  transforms_type transforms_;
};

template <typename CurrentIfca, typename Transform>
inline typename std::enable_if_t<
    is_ifca_interface_v<CurrentIfca> && is_transform_expression_v<Transform> &&
        !std::is_same_v<typename CurrentIfca::transforms_type,
                        transform_chain_t<>> &&
        !std::is_lvalue_reference_v<Transform>,
    IfcaImpl<CurrentIfca, Transform>>
operator+(CurrentIfca&& current_ifca, Transform&& new_transform) {
  return IfcaImpl<CurrentIfca, Transform>(FWD(current_ifca),
                                          FWD(new_transform));
}

}  // namespace ifca

#endif  // IFCA_H
