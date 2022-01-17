#ifndef FILTER_H
#define FILTER_H

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "helpers/functionTraits.hpp"
#include "transformExpression.hpp"

namespace ifca {

template <typename Predicate, typename Deleter = void, typename Enable = void>
struct FilterTransform;

template <typename Predicate, typename Deleter>
class FilterTransform<
    Predicate, Deleter,
    std::enable_if_t<
        std::is_convertible_v<typename function_traits<Predicate>::return_type,
                              bool> &&
        std::is_convertible_v<
            typename function_traits<Predicate>::template arg<0>,
            typename function_traits<Deleter>::template arg<0>>>>
    : public CrtpImpl<FilterTransform, Predicate, TransformExpression> {
 public:
  using base_type = CrtpImpl<FilterTransform, Predicate, TransformExpression>;
  using exact_type = typename base_type::exact_type;
  using input_type = typename function_traits<Predicate>::template arg<0>;
  using output_type = typename function_traits<Predicate>::template arg<0>;

  // TODO: add forwarding
  explicit FilterTransform(Predicate& predicate, Deleter& deleter)
      : predicate_(predicate), deleter_(deleter){};

  template <typename Chunk, typename ResolveCallback, typename RejectCallback,
            typename NextTransform, typename... TransformChain>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve,
                  RejectCallback&& reject, NextTransform&& next,
                  TransformChain&&... transforms) {
    if (predicate_(chunk)) {
      next(FWD(chunk), FWD(resolve), FWD(reject), FWD(transforms)...);
    } else {
      deleter_(FWD(chunk));
      reject();
    }
  }

  template <typename Chunk, typename ResolveCallback, typename RejectCallback>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve,
                  RejectCallback&& reject) {
    if (predicate_(chunk)) {
      resolve(FWD(chunk));
    } else {
      deleter_(FWD(chunk));
      reject();
    }
  }

 private:
  Predicate predicate_;
  Deleter deleter_;
};

template <typename Predicate, typename Deleter>
auto Filter(Predicate& predicate, Deleter deleter) {
  return FilterTransform<Predicate, Deleter>(predicate, deleter);
}

template <typename Predicate>
class FilterTransform<
    Predicate, void,
    std::enable_if_t<std::is_convertible_v<
        typename function_traits<Predicate>::return_type, bool>>>
    : public CrtpImpl<FilterTransform, Predicate, TransformExpression> {
 public:
  using base_type = CrtpImpl<FilterTransform, Predicate, TransformExpression>;
  using exact_type = typename base_type::exact_type;
  using input_type = typename function_traits<Predicate>::template arg<0>;
  using output_type = typename function_traits<Predicate>::template arg<0>;

  // TODO: add forwarding
  explicit FilterTransform(Predicate& predicate) : predicate_(predicate){};

  template <typename Chunk, typename ResolveCallback, typename RejectCallback,
            typename NextTransform, typename... TransformChain>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve,
                  RejectCallback&& reject, NextTransform&& next,
                  TransformChain&&... transforms) {
    if (predicate_(chunk)) {
      next(FWD(chunk), FWD(resolve), FWD(reject), FWD(transforms)...);
    } else {
      reject();
    }
  }

  template <typename Chunk, typename ResolveCallback, typename RejectCallback>
  void operator()(Chunk&& chunk, ResolveCallback&& resolve,
                  RejectCallback&& reject) {
    if (predicate_(chunk)) {
      resolve(FWD(chunk));
    } else {
      reject();
    }
  }

 private:
  Predicate predicate_;
};

template <typename Predicate>
auto filter(Predicate& predicate) {
  return FilterTransform<Predicate>(predicate);
}

}  // namespace ifca

#endif  // FILTER_H