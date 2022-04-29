#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include <sstream>

#include "loggerImpl.hpp"

enum class LogLvl { error, warning, info, debug };

struct MessageLogger {
  LogLvl lvl_;
  int line_;
  const char* func_;
  const std::string& time_;

  MessageLogger(LogLvl lvl, int line, const char* func, const std::string& time)
      : lvl_(lvl), line_(line), func_(func), time_(time) {}

  template <typename T>
  static void log(LogLvl lvl, int line, const char* func,
                  const std::string& time, T&& message) {
    switch (lvl) {
      case LogLvl::debug:
        LoggerImpl::GetInstance().debug(line, func, time,
                                     std::forward<T>(message));
        break;
      case LogLvl::info:
        LoggerImpl::GetInstance().info(line, func, time,
                                        std::forward<T>(message));
        break;
      case LogLvl::warning:
        LoggerImpl::GetInstance().warning(line, func, time,
                                       std::forward<T>(message));
        break;
      case LogLvl::error:
        LoggerImpl::GetInstance().error(line, func, time,
                                     std::forward<T>(message));
        break;
      default:
        break;
    }
  }
};

struct LogMessage {
  std::stringstream ss;
  const MessageLogger& logger_;

  LogMessage(const MessageLogger& logger) : logger_(logger){};
  LogMessage(const LogMessage&) = delete;
  LogMessage& operator=(const LogMessage&) = delete;
  LogMessage& operator=(LogMessage&&) = delete;
  LogMessage(LogMessage&& buf) noexcept : logger_(buf.logger_){};

  template <typename T>
  LogMessage& operator<<(T&& message) {
    ss << std::forward<T>(message);
    return *this;
  }

  ~LogMessage() {
    MessageLogger::log(logger_.lvl_, logger_.line_, logger_.func_,
                       logger_.time_, ss);
  }
};

template <typename T>
LogMessage operator<<(MessageLogger&& logger, T&& message) {
  LogMessage buf(logger);
  buf.ss << std::forward<T>(message);
  return buf;
}

#endif  // LOG_MESSAGE_H