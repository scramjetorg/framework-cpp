#ifndef IFCA_H
#define IFCA_H

#include <type_traits>

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "ifca/ifcaMethods.hpp"
#include "ifca/transform/transformExpression.hpp"
#include "ifca/transform/isTransformExpression.hpp"
#include "transformChain.hpp"
#include "types.hpp"
#include "ifca/maxParallel.hpp"

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
                            !is_transform_expression_v<OutputType>>>
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
           std::enable_if_t<is_transform_expression_v<NextTransform>>>
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
// typename std::enable_if_t<>
auto
 operator+(CurrentIfca&& current_ifca, NewTransform&& new_transform) {
  return Ifca<CurrentIfca, NewTransform>(FWD(current_ifca), FWD(new_transform));
}

// template <typename CurrentIfca, typename NewTransfrom>
// typename std::enable_if_t<
//     (
//         // IsTransformExpression<CurrentIfca>::value ||
//         (isIfcaInterface<CurrentIfca>::value &&
//          std::is_same_v<Ifca<decltype(CurrentIfca::current_transform_),
//                              decltype(CurrentIfca::next_transfrom_), bool>,
//                         typename CurrentIfca::Impl>)) &&
//         IsTransformExpression<NewTransfrom>::value,
//     // Ifca<CurrentIfca, NewTransfrom, bool>
//     bool>
// operator+(CurrentIfca&& current_ifca,
//           NewTransfrom&& new_transform) {
//   Ifca<CurrentIfca, NewTransfrom, bool>(std::move(current_ifca),
//   FWD(new_transform)); return true;
// }

// /**
//  * @brief Template specialization for empty ifca
//  *
//  * Template pattern: Ifca<InputType, OutputType, void>
//  *
//  * @tparam InputType Input type for ifca i.e. type recived .from()
//  * @tparam OutputType Return type for ifca
//  */
// template <typename InputType, typename OutputType>
// // TODO: add check if InputType and OutputType is not ifca
// class Ifca<InputType, OutputType,
//            std::enable_if_t<!IsTransformExpression<InputType>::value &&
//                             !IsTransformExpression<OutputType>::value>>

//     : public IfcaMethods<Ifca<InputType, OutputType>, InputType, OutputType>
//     {
//  public:
//   using Impl = Ifca<InputType, OutputType>;
//   using input_type = InputType;
//   using result_type = OutputType;
//   using base_type = IfcaMethods<Impl, input_type, result_type>;

//   Ifca(unsigned int max_parallel = maxParallel())
//       : base_type(state_), state_(max_parallel) {
//     LOG_DEBUG() << "Empty Ifca created";
//   };
//   Ifca(const Ifca&) = delete;
//   Ifca(Ifca&& other) : state_(std::move(other.state_)) {
//     LOG_DEBUG() << "Empty Ifca move constructor";
//   };
//   Ifca& operator=(const Ifca&) = delete;
//   Ifca& operator=(Ifca&& other) {
//     state_ = std::move(other.state_);
//     return *this;
//   };
//   ~Ifca() { LOG_DEBUG() << "Empty Ifca removed"; };

//   template <typename Transform>
//   Ifca<Transform, void, void> operator+(Transform&& transform) {
//     LOG_DEBUG() << "Empty Ifca operator+";
//     return Ifca<Transform, void>(FWD(transform), std::move(state_));
//   }

//  protected:
//   // TODO: check copy elision for return chunk
//   template <typename Chunk>
//   Chunk operator()(Chunk&& chunk) {
//     LOG_DEBUG() << "Empty Ifca operator()";
//     return FWD(chunk);
//   }

//  private:
//   friend base_type;
//   IfcaState state_;
// };

// /**
//  * @brief Template specialization for single transform ifca
//  *
//  * Template pattern: Ifca<InitTransform, void, void>
//  *
//  * @tparam InitTransform First transform in chain
//  */
// template <typename InitTransform>
// class Ifca<InitTransform, void,
//            std::enable_if_t<IsTransformExpression<InitTransform>::value>>
//     : public IfcaMethods<Ifca<InitTransform, void>,
//                          typename InitTransform::input_type,
//                          typename InitTransform::result_type> {
//  public:
//   using Impl = Ifca<InitTransform>;
//   using input_type =
//       typename std::remove_reference_t<InitTransform>::input_type;
//   using result_type =
//       typename std::remove_reference_t<InitTransform>::result_type;
//   using base_type = IfcaMethods<Impl, input_type, result_type>;
//   // using exact_type = typename base_type::exact_type;

//   Ifca(InitTransform&& init_transform,
//        unsigned int max_parallel = maxParallel())
//       : base_type(state_),
//         init_transform_(init_transform),
//         state_(max_parallel) {
//     LOG_DEBUG() << "Single Ifca created";
//   }

//   Ifca(InitTransform&& init_transform, IfcaState&& shared)
//       : base_type(state_),
//         init_transform_(init_transform),
//         state_(std::move(shared)) {
//     LOG_DEBUG() << "Single Ifca created with shared";
//   }

//   ~Ifca() { LOG_DEBUG() << "Single Ifca removed"; };

//   // TODO: add SFINAE
//   template <typename Transform>
//   Ifca<InitTransform, Transform, bool> operator+(Transform&& transform) {
//     LOG_DEBUG() << "Single Ifca operator+";
//     return Ifca<InitTransform, Transform, bool>(
//         FWD(init_transform_), FWD(transform), std::move(state_));
//   }

//  protected:
//   template <typename Chunk>
//   result_type operator()(Chunk&& chunk) {
//     LOG_DEBUG() << "Single Ifca operator()";
//     return init_transform_(FWD(chunk));
//   }

//  private:
//   friend base_type;
//   InitTransform init_transform_;
//   IfcaState state_;
// };

/**
 * @brief Template specialization for ifca transform chain
 *
 * Template pattern: Ifca<CurrentTransform, NextTransfrom, bool>
 *
 * @tparam CurrentTransform InitTransform or Current transform chain
 * @tparam NextTransfrom Last transform in chain
 */
// template <typename CurrentTransform, typename NextTransfrom>
// // TODO: std::remove_reference_t - check if needed in SFINAE after rebuild of
// // operator()
// class Ifca<
//     CurrentTransform, NextTransfrom, bool>
//     // std::enable_if_t<
//     //     (IsTransformExpression<CurrentTransform>::value ||
//     // isIfcaInterface<std::remove_reference_t<CurrentTransform>>::value) &&
//     //         IsTransformExpression<NextTransfrom>::value,
//     //     bool>>
//     : public IfcaMethods<
//           Ifca<CurrentTransform, NextTransfrom, bool>,
//           typename std::remove_reference_t<CurrentTransform>::input_type,
//           typename std::remove_reference_t<NextTransfrom>::result_type> {
//  public:
//   using Impl = Ifca<CurrentTransform, NextTransfrom, bool>;
//   using input_type =
//       typename std::remove_reference_t<CurrentTransform>::input_type;
//   using result_type =
//       typename std::remove_reference_t<NextTransfrom>::result_type;
//   using base_type = IfcaMethods<Impl, input_type, result_type>;

//   Ifca(CurrentTransform&& current_transform, NextTransfrom&& next_transfrom,
//        unsigned int max_parallel = maxParallel())
//       : base_type(max_parallel),
//         current_transform_(FWD(current_transform)),
//         next_transfrom_(FWD(next_transfrom))
//         // state_(max_parallel)
//         {
//     LOG_DEBUG() << "Ifca created"
//                 << IsTransformExpression<CurrentTransform>::value;
//   }

//   // Ifca(CurrentTransform&& current_transform, NextTransfrom&&
//   next_transfrom,
//   //      IfcaState&& shared)
//   //     : base_type(state_),
//   //       current_transform_(current_transform),
//   //       next_transfrom_(next_transfrom),
//   //       state_(std::move(shared)) {
//   //   LOG_DEBUG()
//   //       << "Ifca created with shared"
//   //       <<
//   isIfcaInterface<std::remove_reference_t<CurrentTransform>>::value;
//   // }
//   ~Ifca() { LOG_DEBUG() << "Ifca removed"; };

//   template <typename Value>
//   result_type operator()(Value&& value) {
//     LOG_DEBUG() << "Ifca operator()";
//     return next_transfrom_(current_transform_(FWD(value)));
//   }

// template <typename Transform>
// auto operator+(Transform&& transform) {
//   LOG_DEBUG() << "Ifca operator+";

//   return Ifca<decltype(*this), Transform, bool>(*this, FWD(transform),
//                                                 std::move(state_));
// }

//  private:
// friend base_type;
// CurrentTransform current_transform_;
// NextTransfrom next_transfrom_;
// IfcaState state_;
// };

// /**
//  * @brief Operator+ for ifca transform chain (Ifca<CurrentTransform,
//  * NextTransfrom, bool> only)
//  *
//  * @tparam CurrentIfca
//  * @tparam NewTransfrom
//  */
// template <typename CurrentIfca, typename NewTransfrom>
// typename std::enable_if_t<
//     (
//         // IsTransformExpression<CurrentIfca>::value ||
//         (isIfcaInterface<CurrentIfca>::value &&
//          std::is_same_v<Ifca<decltype(CurrentIfca::current_transform_),
//                              decltype(CurrentIfca::next_transfrom_), bool>,
//                         typename CurrentIfca::Impl>)) &&
//         IsTransformExpression<NewTransfrom>::value,
//     // Ifca<CurrentIfca, NewTransfrom, bool>
//     bool>
// operator+(CurrentIfca&& current_ifca,
//           NewTransfrom&& new_transform) {
//   Ifca<CurrentIfca, NewTransfrom, bool>(std::move(current_ifca),
//   FWD(new_transform)); return true;
// }

}  // namespace ifca

#endif  // IFCA_H
