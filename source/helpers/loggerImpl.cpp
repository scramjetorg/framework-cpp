#include <iostream>

#include "helpers/Logger/loggerImpl.hpp"
#include "helpers/Logger/terminalColorCodes.hpp"

Logger* Logger::instance_ = nullptr;
std::mutex Logger::instance_mutex_;

Logger::Logger() {
#if LOG_TYPE == LOG_TYPE_FILE
  const std::string logFileName = "ifca.log";
  file_.open(logFileName.c_str(), std::ios::out | std::ios::app);
  if (!file_.is_open()) throw "Unable to open log file";
  file_ << "\n==== " << __DATE__ << " " << __TIME__ << " ====" << std::endl;
#endif
}

Logger::~Logger() {
#if LOG_TYPE == LOG_TYPE_FILE
  file_.close();
#endif
}

Logger* Logger::GetInstance() {
  std::lock_guard<std::mutex> lock(instance_mutex_);
  if (instance_ == nullptr) {
    instance_ = new Logger();
  }
  return instance_;
}

void Logger::error(int line, const char* func, const std::string& time,
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
}

void Logger::warning(int line, const char* func, const std::string& time,
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
}

void Logger::info(int line, const char* func, const std::string& time,
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
}

void Logger::debug(int line, const char* func, const std::string& time,
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
}