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
template <typename In, typename Out = In, typename IsTransformChain = void>
struct Ifca;

template <typename InputType>
class Ifca<InputType, InputType,
           std::enable_if_t<!is_transform_expression_v<InputType> &&
                            !is_ifca_interface_v<InputType> &&
                            !std::is_void_v<InputType>>>
    : public IfcaMethods<Ifca<InputType, InputType>, InputType, InputType> {
 public:
  using Impl = Ifca<InputType, InputType>;
  using input_type = InputType;
  using output_type = InputType;
  using base_type = IfcaMethods<Impl, input_type, output_type>;
  using transforms_type = transform_chain_t<>;

  explicit Ifca(unsigned int max_parallel = maxParallel())
      : base_type(max_parallel){};

  template <typename Chunk>
  void operator()(Chunk&& chunk,
                  std::future<void>&& previous_processing_future) {
    this->onResolve(FWD(chunk), std::move(previous_processing_future));
  }

  template <typename NewTransform>
  typename std::enable_if_t<is_transform_expression_v<NewTransform>,
                            Ifca<NewTransform, void>>
  operator+(NewTransform&& new_transform) {
    return Ifca<NewTransform, void>(FWD(*this), FWD(new_transform));
  }

  template <typename, typename, typename>
  friend class Ifca;

 private:
  transforms_type transforms_;
};

///////////////////////////////

template <typename FirstTransform>
class Ifca<FirstTransform, void,
           std::enable_if_t<is_transform_expression_v<FirstTransform>>>
    : public IfcaMethods<Ifca<FirstTransform, void>,
                         typename FirstTransform::input_type,
                         typename FirstTransform::output_type> {
 public:
  using Impl = Ifca<FirstTransform, void>;
  using input_type = typename FirstTransform::input_type;
  using output_type = typename FirstTransform::output_type;
  using base_type = IfcaMethods<Impl, input_type, output_type>;
  using transforms_type = std::tuple<FirstTransform>;

  // explicit Ifca(unsigned int max_parallel = maxParallel())
  //     : base_type(max_parallel){};

  template <typename CurrentIfca>
  explicit Ifca(CurrentIfca&& currentIfca, FirstTransform&& firstTransform)
      : base_type(std::move(currentIfca)),
        transforms_(transforms_type{std::move(firstTransform)}) {}
  // transforms_(ForwardTransformChain<typename CurrentIfca::transforms_type,
  //                                   FirstTransform>(
  //     std::move(currentIfca.transforms_), std::move(nextTransform))) {}

  template <typename Chunk>
  void operator()(Chunk&& chunk,
                  std::future<void>&& previous_processing_future) {
    auto&& onResolveCallback =
        [&previous_processing_future, this](
            decltype(std::forward<output_type>(chunk)) resolvedValue) -> void {
      this->onResolve(FWD(resolvedValue),
                      std::move(previous_processing_future));
    };

    auto&& onRejectCallback = [this] { this->onReject(); };
    std::get<0>(transforms_)(FWD(chunk), FWD(onResolveCallback),
                             FWD(onRejectCallback));
  }

  template <typename, typename, typename>
  friend class Ifca;

 private:
  transforms_type transforms_;
};

///////////////////////////////

template <typename CurrentIfca, typename NextTransform>
class Ifca<CurrentIfca, NextTransform,
           std::enable_if_t<is_ifca_interface_v<CurrentIfca> &&
                            is_transform_expression_v<NextTransform>>>
    : public IfcaMethods<Ifca<CurrentIfca, NextTransform>,
                         typename CurrentIfca::input_type,
                         typename NextTransform::output_type> {
 public:
  using Impl = Ifca<CurrentIfca, NextTransform>;
  using input_type = typename CurrentIfca::input_type;
  using output_type = typename NextTransform::output_type;
  using base_type = IfcaMethods<Impl, input_type, output_type>;
  using transforms_type =
      transform_chain_t<NextTransform, typename CurrentIfca::transforms_type>;

  explicit Ifca(CurrentIfca&& currentIfca, NextTransform&& nextTransform)
      : base_type(std::move(currentIfca)),
        transforms_(ForwardTransformChain<typename CurrentIfca::transforms_type,
                                          NextTransform>(
            std::move(currentIfca.transforms_), std::move(nextTransform))){};

  template <typename Chunk>
  void operator()(Chunk&& chunk,
                  std::future<void>&& previous_processing_future) {
    runTransformsHelper(
        FWD(chunk), std::move(previous_processing_future),
        std::make_index_sequence<std::tuple_size<transforms_type>{}>{});
  }

 protected:
  template <typename Chunk, std::size_t... Is>
  void runTransformsHelper(Chunk&& chunk,
                           std::future<void>&& previous_processing_future,
                           std::index_sequence<Is...>) {
    runTransforms(FWD(chunk), std::move(previous_processing_future),
                  std::get<Is>(transforms_)...);
  }

  template <typename Chunk, typename FirstTransform,
            typename... RestOfTransforms>
  void runTransforms(Chunk&& chunk,
                     std::future<void>&& previous_processing_future,
                     FirstTransform&& firstTransform,
                     RestOfTransforms&&... restOfTransforms) {
    using namespace std::placeholders;

    auto&& onResolveCallback =
        [&previous_processing_future, this](
            decltype(std::forward<output_type>(chunk)) resolvedValue) -> void {
      this->onResolve(FWD(resolvedValue),
                      std::move(previous_processing_future));
    };

    // TODO: change to labmda
    // auto&& onRejectCallback = std::bind(&Impl::onReject, this);
    auto&& onRejectCallback = [this] { this->onReject(); };
    firstTransform(FWD(chunk), FWD(onResolveCallback), FWD(onRejectCallback),
                   FWD(restOfTransforms)...);
    // previous_processing_future.wait();
  }

  template <typename, typename, typename>
  friend class Ifca;

 private:
  transforms_type transforms_;
};

template <typename CurrentIfca, typename NewTransform>
typename std::enable_if_t<
    is_ifca_interface_v<CurrentIfca> &&
        is_transform_expression_v<NewTransform> &&
        !std::is_same_v<typename CurrentIfca::transforms_type,
                        transform_chain_t<>>,
    Ifca<CurrentIfca, NewTransform>>
operator+(CurrentIfca&& current_ifca, NewTransform&& new_transform) {
  return Ifca<CurrentIfca, NewTransform>(FWD(current_ifca), FWD(new_transform));
}

}  // namespace ifca

#endif  // IFCA_H
