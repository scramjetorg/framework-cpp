#ifndef LOGGER_H
#define LOGGER_H

#define LOG_LEVEL_ALL 4
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_NONE 0

#include "emptyLogger.hpp"
#include "messageLogger.hpp"
#include "logMessage.hpp"
#include "time.hpp"

#if LOG_LVL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG() \
  MessageLogger(LogLvl::debug, __LINE__, __func__, time_now_str())
#else
#define LOG_DEBUG() EmptyLogger()
#endif

#if LOG_LVL >= LOG_LEVEL_INFO
#define LOG_INFO() \
  MessageLogger(LogLvl::info, __LINE__, __func__, time_now_str())
#else
#define LOG_INFO() EmptyLogger()
#endif

#if LOG_LVL >= LOG_LEVEL_WARNING
#define LOG_WARNING() \
  MessageLogger(LogLvl::warning, __LINE__, __func__, time_now_str())
#else
#define LOG_WARNING() EmptyLogger()
#endif

#if LOG_LVL >= LOG_LEVEL_ERROR
#define LOG_ERROR() \
  MessageLogger(LogLvl::error, __LINE__, __func__, time_now_str())
#else
#define LOG_ERROR() EmptyLogger()
#endif

#endif  // LOGGER_H