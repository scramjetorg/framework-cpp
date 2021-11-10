#ifndef EMPTY_LOGGER_H
#define EMPTY_LOGGER_H

struct EmptyLogger {
  template <typename T>
  EmptyLogger operator<<(T&&) {
    return {};
  }
};

#endif  // EMPTY_LOGGER_H