#ifndef TRANSFORM_CHAIN_H
#define TRANSFORM_CHAIN_H

#include <tuple>

//TODO: add namespace

/**
 * @brief Helper class to deduce type of  chained transforms
 */
template <typename NextTransform = void, typename CurrentTransforms = void>
struct transform_chain;

/**
 * @brief Specialization for no transforms
 */
template <>
struct transform_chain<> {
  using type = std::tuple<>;
};

/**
 * @brief Specialization for chain of transforms
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

template <typename TransformChain, size_t S>
struct unfoldTransformsImpl {
  template <typename Chunk>
  auto operator()(TransformChain& transform, Chunk&& chunk) {
    return std::get<S>(transform)(
        unfoldTransformsImpl<TransformChain, S - 1>()(transform, FWD(chunk)));
  }
};

template <typename TransformChain>
struct unfoldTransformsImpl<TransformChain, 0> {
  template <typename Chunk>
  auto operator()(TransformChain& transform, Chunk&& chunk) {
    return std::get<0>(transform)(FWD(chunk));
  }
};

/**
 * @brief Helper method to unfold and run transform chain for given chunk.
 *
 * Unfolds chain of transforms i.e. for 3 transforms [T0, T1, T2] returns
 * T2(T1(T0(chunk)));
 *
 * @param transformChain transfom chain to unfold
 * @param chunk piece of data to run transforms on.
 * @return result of transformations of given chunk
 */
template <typename TransformChain, typename Chunk>
auto unfoldTransforms(TransformChain& transformChain, Chunk& chunk) {
  return unfoldTransformsImpl<TransformChain,
                              std::tuple_size_v<TransformChain> - 1U>()(
      transformChain, chunk);
}

#endif  // TRANSFORM_CHAIN_H