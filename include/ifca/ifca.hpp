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
#include "ifca/transform/transformBase.hpp"
#include "types.hpp"

// TODO: doxygen docs
// TODO: LOG class
// TODO: add valgrind task
// TODO: add clang-tidy
namespace ifca {

class Ifca {
 public:
  explicit Ifca(unsigned int max_parallel);
  Ifca(const Ifca&) = delete;
  Ifca(Ifca&&) = delete;
  Ifca& operator=(const Ifca&) = delete;
  Ifca& operator=(Ifca&&) = delete;
  ~Ifca();

  // future set - no drain | future waitnig = drain
  drain_sfuture Write(chunk_intype chunk);  // TODO: handle rvalue argumenst
  chunk_future Read();
  void End();

  void addTransform(std::function<chunk_outtype(chunk_intype)>&& transform);

 private:
  DrainState drain_state_;

  ThreadList<chunk_promise> processing_promises_;

  std::future<void> processing_future_;

  std::list<chunk_promise> read_ahead_promises_;
  std::list<chunk_future> read_futures_;

  bool ended_;

  std::vector<std::function<chunk_outtype(chunk_intype)>> transforms_;
};

}  // namespace ifca

#endif  // IFCA_H
