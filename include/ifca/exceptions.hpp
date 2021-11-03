#include <exception>
#include <future>

class MultipleEnd : public std::exception {
 public:
  MultipleEnd() noexcept {};
  virtual ~MultipleEnd() = default;
  virtual const char* what() const noexcept override {
    return "Multiple calls to End()";
  };
};

class WriteAfterEnd : public std::exception {
 public:
  WriteAfterEnd() noexcept {};
  virtual ~WriteAfterEnd() = default;
  virtual const char* what() const noexcept override {
    return "Called Write() after End()";
  };
};

class ReadEnd : public std::exception {
 public:
  ReadEnd() noexcept {};
  virtual ~ReadEnd() = default;
  virtual const char* what() const noexcept override {
    return "Reading from empty ended Ifca";
  };
};

class UncallableFunction : public std::exception {
 public:
  UncallableFunction() noexcept {};
  virtual ~UncallableFunction() = default;
  virtual const char* what() const noexcept override {
    return "Transform function must be callable";
  };
};