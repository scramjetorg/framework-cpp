#include "ifca/transform/transformBase.hpp"

namespace ifca {

void TransformBase::SetNextTransform(
    std::unique_ptr<TransformBase> nextTransform) {
  nextTransform_ = std::move(nextTransform);
}

}  // namespace ifca