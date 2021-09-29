#ifndef IFCA_H
#define IFCA_H

#include <future>
#include <list>
#include <mutex>

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

  void addTransform(std::unique_ptr<TransformBase> transform);

 protected:
  bool IsDrainLvl();
  void CreateDrain();
  void SetClearDrain();

 private:
  unsigned int max_parallel_;

  drain_promise drain_promise_;
  drain_sfuture drain_sfuture_;

  std::mutex processing_futures_mutex;
  std::list<std::future<void>> processing_futures_;
  decltype(processing_futures_)::iterator processing_futures_current;

  std::list<chunk_promise> read_ahead_promises_;

  std::mutex processing_promises_mutex;
  std::list<chunk_promise> processing_promises_;

  std::list<chunk_future> read_futures_;

  bool ended_;

  std::unique_ptr<TransformBase> transform_;  // TODO: create class
};

}  // namespace ifca

#endif  // IFCA_H
