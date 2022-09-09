#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <future>

class MultipleEnd : public std::exception {
 public:
  MultipleEnd() noexcept {};
  virtual const char* what() const noexcept override {
    return "Multiple calls to End()";
  };
};

class WriteAfterEnd : public std::exception {
 public:
  WriteAfterEnd() noexcept {};
  virtual const char* what() const noexcept override {
    return "Called Write() after End()";
  };
};

class ReadEnd : public std::exception {
 public:
  ReadEnd() noexcept {};
  virtual const char* what() const noexcept override {
    return "Reading from empty ended Ifca";
  };
};

#endif  // EXCEPTIONS_H