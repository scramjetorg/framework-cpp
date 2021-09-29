#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <functional>

#include "transformBase.hpp"

namespace ifca {

class Transform : public TransformBase {
 public:
  Transform() = default;
  explicit Transform(std::function<std::string(std::string)>& func);

  virtual void Run(std::list<chunk_promise>& processing_promises_,
                   std::mutex& processing_promises_mutex,
                   const std::future<void>& previous_ready,
                   std::string&& chunk);

 private:
  std::function<std::string(std::string)> func_;
};

}  // namespace ifca

#endif  // TRANSFORM_H
