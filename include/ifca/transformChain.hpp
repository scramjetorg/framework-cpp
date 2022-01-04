#ifndef TRANSFORM_CHAIN_H
#define TRANSFORM_CHAIN_H

#include <tuple>

// TODO: add namespace

/**
 * @brief Helper class to deduce type of  chained transforms
 */
template <typename NextTransform = void, typename CurrentTransforms = void>
struct transform_chain;

/**
 * @brief Empty transform chain
 */
template <>
struct transform_chain<> {
  using type = std::tuple<>;
};

/**
 * @brief Single transform in chain
 */
template <typename FirstTransform>
struct transform_chain<FirstTransform, void> {
  using type = std::tuple<FirstTransform>;
};

/**
 * @brief Multiple transforms in chain
 *
 * @tparam NextTransform New transform to attach to chain
 * @tparam CurrentTransforms
 */
template <typename NextTransform, typename... CurrentTransforms>
struct transform_chain<NextTransform, std::tuple<CurrentTransforms...>> {
  using type = std::tuple<CurrentTransforms..., NextTransform>;
};

/**
 * @brief Helper alias template to get type of transform chain.
 */
template <typename T1 = void, typename T2 = void>
using transform_chain_t = typename transform_chain<T1, T2>::type;

template <typename ExistingTransformChain, typename NewTransfrom,
          std::size_t... I>
auto ForwardTransformChainImpl(ExistingTransformChain&& existingTransformChain,
                               NewTransfrom&& newTransfrom,
                               std::index_sequence<I...>) {
  return std::make_tuple(std::get<I>(FWD(existingTransformChain))...,
                         FWD(newTransfrom));
}

/**
 * @brief Helper method to create new tuple containg current transform chain and
 * new transform to add
 *
 * @param existingTransformChain Current chain of transforms
 * @param newTransfrom New transform to add to transform chain
 * @return Tuple collection of current transfomrs and new transform
 */
template <typename ExistingTransformChain, typename NewTransfrom,
          typename Indices = std::make_index_sequence<
              std::tuple_size_v<ExistingTransformChain>>>
auto ForwardTransformChain(ExistingTransformChain&& existingTransformChain,
                           NewTransfrom&& newTransfrom) {
  return ForwardTransformChainImpl<ExistingTransformChain, NewTransfrom>(
      FWD(existingTransformChain), FWD(newTransfrom), Indices{});
}

#endif  // TRANSFORM_CHAIN_H