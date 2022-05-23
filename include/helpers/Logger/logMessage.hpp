#include <sstream>

#include "messageLogger.hpp"

struct LogMessage {
  const MessageLogger& logger_;
  std::stringstream ss;

  LogMessage(const MessageLogger& logger) : logger_(logger){};
  LogMessage(const LogMessage&) = delete;
  LogMessage& operator=(const LogMessage&) = delete;
  LogMessage& operator=(LogMessage&&) = delete;
  LogMessage(LogMessage&& buf) noexcept
      : logger_(buf.logger_), ss(std::move(buf.ss)){};

  template <typename T>
  LogMessage& operator<<(T&& message) {
    ss << std::forward<T>(message);
    return *this;
  }

  ~LogMessage() {
    if (ss.tellp() != 0)
      logger_.log(logger_.lvl_, logger_.line_, logger_.func_, logger_.time_,
                  ss);
  }
};

template <typename T>
LogMessage operator<<(MessageLogger&& logger, T&& message) {
  // FIXME: implicit move forces ~LogMessage() with empty ss
  LogMessage buf(logger);
  buf.ss << std::forward<T>(message);
  return buf;
}
