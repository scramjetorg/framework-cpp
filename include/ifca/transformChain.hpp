#ifndef TRANSFORM_CHAIN_H
#define TRANSFORM_CHAIN_H

#include <tuple>

namespace detail {

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

}  // namespace detail

#endif  // TRANSFORM_CHAIN_H