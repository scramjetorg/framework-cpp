#ifndef IFCA_H
#define IFCA_H

#include <atomic>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <vector>

#include "helpers/threadList.hpp"
#include "ifca/drain.hpp"
#include "types.hpp"

// TODO: doxygen docs
namespace ifca {

class Ifca {
 public:
  explicit Ifca(unsigned int max_parallel = maxParallel());
  Ifca(const Ifca&) = delete;
  Ifca(Ifca&&) = delete;
  Ifca& operator=(const Ifca&) = delete;
  Ifca& operator=(Ifca&&) = delete;
  ~Ifca();

  // future set - no drain | future waitnig = drain
  drain_sfuture Write(chunk_intype chunk);  // TODO: handle rvalue argumenst
  chunk_future Read();
  void End();

  void addTransform(transform_type& transform);

 protected:
  static unsigned int maxParallel();
  void setReadEnd(chunk_promise& promise);

 private:
  DrainState drain_state_;

  ThreadList<chunk_promise> processing_promises_;

  std::future<void> last_processing_future_;

  std::list<chunk_promise> read_ahead_promises_;
  std::list<chunk_future> read_futures_;

  bool ended_;

  std::vector<transform_type> transforms_;
};

}  // namespace ifca

#endif  // IFCA_H
