#ifndef TRANSFORM_BASE_H
#define TRANSFORM_BASE_H

#include <atomic>
#include <memory>

#include "../types.hpp"
#include "helpers/threadList.hpp"

namespace ifca {

class TransformBase {
 public:
  TransformBase() = default;
  TransformBase(const TransformBase&) = delete;
  TransformBase(TransformBase&& other) = default;
  TransformBase& operator=(const TransformBase&) = delete;
  TransformBase& operator=(TransformBase&& other) = default;
  virtual ~TransformBase() = default;

  virtual void Run(ThreadList<chunk_promise>& processing_promises_,
                   const std::future<void>& previous_ready,
                   std::string&& chunk) = 0;
  void SetNextTransform(std::unique_ptr<TransformBase> nextTransform);
  TransformBase* GetNextTransform() { return nextTransform_.get(); };

 private:
  std::unique_ptr<TransformBase> nextTransform_;
};

}  // namespace ifca

#endif  // TRANSFORM_BASE_H
