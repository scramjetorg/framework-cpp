#ifndef LOGGER_IMPL_H
#define LOGGER_IMPL_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

#include "terminalColorCodes.hpp"

#define LOG_TYPE_CONSOLE 1
#define LOG_TYPE_FILE 2

class LoggerImpl {
 public:
  LoggerImpl(const LoggerImpl&) = delete;
  LoggerImpl(LoggerImpl&&) = delete;
  void operator=(const LoggerImpl&) = delete;
  void operator=(LoggerImpl&&) = delete;

  static LoggerImpl& GetInstance() {
    static LoggerImpl logger;
    return logger;
  };

  void error(int line, const char* func, const std::string& time,
             std::stringstream& message) {
    std::lock_guard<std::mutex> ml(m_);

#if LOG_TYPE == LOG_TYPE_CONSOLE
    std::cout << time << " " << func << " (:" << line << ") " << BOLDRED
              << "[ERROR]" << RESET << ": " << message.str() << std::endl;
#elif LOG_TYPE == LOG_TYPE_FILE
    file_ << time << " " << func << " (:" << line << ") "
          << "[ERROR]: " << message.str() << std::endl;
#endif
    message.flush();
  };

  void warning(int line, const char* func, const std::string& time,
               std::stringstream& message) {
    std::lock_guard<std::mutex> ml(m_);
#if LOG_TYPE == LOG_TYPE_CONSOLE
    std::cout << time << " " << func << " (:" << line << ") " << YELLOW
              << "[WARNING]" << RESET << ": " << message.str() << std::endl;
#elif LOG_TYPE == LOG_TYPE_FILE
    file_ << time << " " << func << " (:" << line << ") "
          << "[WARNING]: " << message.str() << std::endl;
#endif
    message.flush();
  };

  void info(int line, const char* func, const std::string& time,
            std::stringstream& message) {
    std::lock_guard<std::mutex> ml(m_);
#if LOG_TYPE == LOG_TYPE_CONSOLE
    std::cout << time << " " << func << " (:" << line << ") " << BOLDBLUE
              << "[INFO]" << RESET << ": " << message.str() << std::endl;
#elif LOG_TYPE == LOG_TYPE_FILE
    file_ << time << " " << func << " (:" << line << ") "
          << "[INFO]: " << message.str() << std::endl;
#endif
    message.flush();
  };

  void debug(int line, const char* func, const std::string& time,
             std::stringstream& message) {
    std::lock_guard<std::mutex> ml(m_);
#if LOG_TYPE == LOG_TYPE_CONSOLE
    std::cout << time << " " << func << " (:" << line << ") " << BOLDGREEN
              << "[DEBUG]" << RESET << ": " << message.str() << std::endl;
#elif LOG_TYPE == LOG_TYPE_FILE
    file_ << time << " " << func << " (:" << line << ") "
          << "[DEBUG]: " << message.str() << std::endl;
#endif
    message.flush();
  };

 protected:
  LoggerImpl() {
#if LOG_TYPE == LOG_TYPE_FILE
    const std::string logFileName = "ifca.log";
    file_.open(logFileName.c_str(), std::ios::out | std::ios::app);
    if (!file_.is_open()) throw "Unable to open log file";
    file_ << "\n==== " << __DATE__ << " " << __TIME__ << " ====" << std::endl;
#endif
  };

  ~LoggerImpl() {
#if LOG_TYPE == LOG_TYPE_FILE
    file_.close();
#endif
  };

 private:
  //static Logger* instance_;
  //static std::mutex instance_mutex_;
  std::mutex m_;

#if LOG_TYPE == LOG_TYPE_FILE
  std::ofstream file_;
#endif
};

#endif  // LOGGER_IMPL_H