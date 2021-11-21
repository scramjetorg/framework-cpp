#ifndef IFCA_H
#define IFCA_H

#include <atomic>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <vector>

#include "helpers/FWD.hpp"
#include "helpers/Logger/logger.hpp"
#include "helpers/threadList.hpp"
#include "ifca/drain.hpp"
#include "ifca/ifcaMethods.hpp"
#include "ifca/ifcaState.hpp"
#include "types.hpp"

// TODO: doxygen docs
namespace ifca {

template <typename In = void, typename Out = void>
class Ifca {
 public:
  Ifca(In&& in, Out&& out, unsigned int max_parallel = maxParallel())
      : in_(in), out_(out), shared_(max_parallel) {
    LOG_DEBUG() << "Ifca created";
  }
  Ifca(In&& in, Out&& out, IfcaState&& shared)
      : in_(in), out_(out), shared_(std::move(shared)) {
    LOG_DEBUG() << "Ifca created with shared";
  }
  ~Ifca() { LOG_DEBUG() << "Ifca removed"; };

  using in_type = In;
  using out_type = Out;
  using input_type = typename std::remove_reference_t<In>::input_type;
  using result_type = typename std::remove_reference_t<In>::result_type;

  template <typename Value>
  result_type operator()(Value&& value) {
    LOG_DEBUG() << "Ifca operator()";
    return out_(in_(FWD(value)));
  }

  template <typename Transform>
  auto operator+(Transform&& transform) {
    LOG_DEBUG() << "Ifca operator+";
    return Ifca<decltype(*this), Transform>(FWD(*this), FWD(transform));
  }

 private:
  In in_;
  Out out_;
  IfcaState shared_;
};

template <typename In>
class Ifca<In, void> {
 public:
  Ifca(In&& in, unsigned int max_parallel = maxParallel())
      : in_(in), shared_(max_parallel) {
    LOG_DEBUG() << "Single Ifca created";
  }

  Ifca(In&& in, IfcaState&& shared) : in_(in), shared_(std::move(shared)) {
    LOG_DEBUG() << "Single Ifca created with shared";
  }

  ~Ifca() { LOG_DEBUG() << "Single Ifca removed"; };

  using in_type = In;
  using out_type = void;
  using input_type = typename std::remove_reference_t<In>::input_type;
  using result_type = typename std::remove_reference_t<In>::result_type;

  template <typename Chunk>
  result_type operator()(Chunk&& chunk) {
    LOG_DEBUG() << "Single Ifca operator()";
    return in_(FWD(chunk));
  }

  template <typename Transform>
  auto operator+(Transform&& transform) {
    LOG_DEBUG() << "Single Ifca operator+";
    return Ifca<In, Transform>(FWD(in_), FWD(transform), std::move(shared_));
  }

 private:
  In in_;
  IfcaState shared_;
};

template <>
class Ifca<void, void> {
 public:
  Ifca(unsigned int max_parallel = maxParallel()) : shared_(max_parallel) {
    LOG_DEBUG() << "Empty Ifca created";
  };
  Ifca(const Ifca&) = delete;
  Ifca(Ifca&& other) : shared_(std::move(other.shared_)) {
    LOG_DEBUG() << "Empty Ifca move constructor";
  };
  Ifca& operator=(const Ifca&) = delete;
  Ifca& operator=(Ifca&& other) {
    shared_ = std::move(other.shared_);
    return *this;
  };
  ~Ifca() { LOG_DEBUG() << "Empty Ifca removed"; };

  using in_type = void;
  using out_type = void;

  drain_sfuture Write(chunk_intype chunk);  // TODO: handle rvalue argumenst
  chunk_future Read();
  void End(){};

  template <typename Chunk>
  Chunk operator()(Chunk&& chunk) {
    LOG_DEBUG() << "Empty Ifca operator()";
    return FWD(chunk);
  }

  template <typename Transform>
  auto operator+(Transform&& transform) {
    LOG_DEBUG() << "Empty Ifca operator+";
    return Ifca<Transform, void>(FWD(transform), std::move(shared_));
  }

 private:
  IfcaState shared_;
};

// class Ifca {
//  public:
//   explicit Ifca(unsigned int max_parallel = maxParallel());
//   Ifca(const Ifca&) = delete;
//   Ifca(Ifca&&) = delete;
//   Ifca& operator=(const Ifca&) = delete;
//   Ifca& operator=(Ifca&&) = delete;
//   ~Ifca();

//   // future set - no drain | future waitnig = drain
//   drain_sfuture Write(chunk_intype chunk);  // TODO: handle rvalue argumenst
//   chunk_future Read();
//   void End();

//   void addTransform(transform_type& transform);

//  protected:
//   static unsigned int maxParallel();
//   void setReadEnd(chunk_promise& promise);

//  private:
//   DrainState drain_state_;
//   std::future<void> last_processing_future_;
//   bool ended_;

//   ThreadList<chunk_promise> processing_promises_;

//   std::list<chunk_promise> read_ahead_promises_;
//   std::list<chunk_future> read_futures_;

//   std::vector<transform_type> transforms_;
// };

}  // namespace ifca

#endif  // IFCA_H
