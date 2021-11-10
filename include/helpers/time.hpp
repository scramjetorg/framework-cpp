#ifndef TIME_H
#define TIME_H

#include <ctime>
#include <sstream>
#include <string>

std::string time_now_str() {
  std::time_t t = std::time(0);
  std::tm* now = std::localtime(&t);
  std::stringstream time_now;
  if (now->tm_hour < 10) time_now << "0";
  time_now << now->tm_hour << ":";
  if (now->tm_min < 10) time_now << "0";
  time_now << now->tm_min << ":";
  if (now->tm_sec < 10) time_now << "0";
  time_now << now->tm_sec;
  return time_now.str();
}

#endif  // TIME_H