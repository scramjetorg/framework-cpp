#ifndef ASYNC_FORWARDER_H
#define ASYNC_FORWARDER_H

/**
 * @brief Reference wraper to bypass implicit lvalue copy when passing
 * forwarding reference to std::async lambda body
 *
 */
template <typename T>
class async_forwarder {
 public:
  async_forwarder(T& t) : val_(std::move(t)) {}
  async_forwarder(async_forwarder const& other) = delete;
  async_forwarder(async_forwarder&& other) : val_(std::move(other.val_)) {}

  operator T&&() { return std::move(val_); }
  operator T&&() const { return std::move(val_); }

 private:
  T val_;
};

// specify for lvalue
template <typename T>
class async_forwarder<T&> {
 public:
  async_forwarder(T& t) : val_(t) {}
  async_forwarder(async_forwarder const& other) = delete;
  async_forwarder(async_forwarder&& other) : val_(other.val_) {}

  operator T&() { return val_; }
  operator T const &() const { return val_; }

 private:
  T& val_;
};

#endif  // ASYNC_FORWARDER_H