#ifndef LOGGER_IMPL_H
#define LOGGER_IMPL_H

#include <fstream>
#include <mutex>
#include <sstream>

#define LOG_TYPE_CONSOLE 1
#define LOG_TYPE_FILE 2

// TODO: move to build configuration
#define LOG_TYPE LOG_TYPE_CONSOLE

class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  void operator=(const Logger&) = delete;
  void operator=(Logger&&) = delete;

  static Logger* GetInstance();

  void error(int line, const char* func, const std::string& time,
             std::stringstream& message);
  void warning(int line, const char* func, const std::string& time,
               std::stringstream& message);
  void info(int line, const char* func, const std::string& time,
            std::stringstream& message);
  void debug(int line, const char* func, const std::string& time,
             std::stringstream& message);

 protected:
  Logger();
  ~Logger();

 private:
  static Logger* instance_;
  static std::mutex instance_mutex_;
  std::mutex m_;

#if LOG_TYPE == LOG_TYPE_FILE
  std::ofstream file_;
#endif
};

#endif  // LOGGER_IMPL_H