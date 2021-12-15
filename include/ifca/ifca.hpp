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

// TODO: doxygen docs
namespace ifca {

/**
 * @brief Base Ifca template
 *
 * @tparam In Begining of transforms of Ifca
 * @tparam Out Last transform in Ifca
 * @tparam IsTransformChain Template flag to help deduce template type
 */
template <typename In, typename Out, typename IsTransformChain = void>
struct Ifca;

template <typename InputType, typename OutputType>
class Ifca<InputType, OutputType,
           std::enable_if_t<!is_transform_expression_v<InputType> &&
                            !is_ifca_interface_v<InputType> &&
                            !is_transform_expression_v<OutputType> &&
                            !is_ifca_interface_v<OutputType>>>
    : public IfcaMethods<Ifca<InputType, OutputType>, InputType, OutputType> {
 public:
  using Impl = Ifca<InputType, OutputType>;
  using input_type = InputType;
  using output_type = OutputType;
  using base_type = IfcaMethods<Impl, input_type, output_type>;
  using transforms_type = transform_chain_t<>;

  Ifca(unsigned int max_parallel = maxParallel()) : base_type(max_parallel) {
    LOG_DEBUG() << "Empty Ifca created";
  };
  ~Ifca() { LOG_DEBUG() << "Empty Ifca deleted"; };

  template <typename Chunk>
  Chunk operator()(Chunk&& chunk) {
    return FWD(chunk);
  }

  template <typename, typename, typename>
  friend class Ifca;

 private:
  transforms_type transforms_;
};

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

  Ifca(CurrentIfca&& currentIfca, NextTransform&& nextTransform)
      : base_type(std::move(currentIfca.state_)),
        transforms_(ForwardTransformChain<typename CurrentIfca::transforms_type,
                                          NextTransform>(
            std::move(currentIfca.transforms_), std::move(nextTransform))) {
    LOG_DEBUG() << "Tuple Ifca created";
  };
  ~Ifca() { LOG_DEBUG() << "Tuple Ifca deleted"; };

  template <typename Chunk>
  auto operator()(Chunk&& chunk) {
    return unfoldTransforms(transforms_, FWD(chunk));
  }

  template <typename, typename, typename>
  friend class Ifca;

 private:
  transforms_type transforms_;
};

template <typename CurrentIfca, typename NewTransform>
typename std::enable_if_t<is_ifca_interface_v<CurrentIfca> &&
                              is_transform_expression_v<NewTransform>,
                          Ifca<CurrentIfca, NewTransform>>
operator+(CurrentIfca&& current_ifca, NewTransform&& new_transform) {
  return Ifca<CurrentIfca, NewTransform>(FWD(current_ifca), FWD(new_transform));
}

}  // namespace ifca

#endif  // IFCA_H
