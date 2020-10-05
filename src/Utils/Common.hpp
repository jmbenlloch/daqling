/**
 * Copyright (C) 2019 CERN
 *
 * DAQling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DAQling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with DAQling. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DAQLING_UTILITIES_COMMON_HPP
#define DAQLING_UTILITIES_COMMON_HPP

#include <limits.h>
#include <pthread.h>
#include <thread>
#include <time.h>
#include <unistd.h>

#include "Types.hpp"

namespace daqling {
namespace utilities {

/*
 * Constants
 * */
class Constant {
public:
  // Constants
  static const u_long Kilo = 1024;
  static const u_long Mega = 1024 * Kilo;
  static const u_long Giga = 1024 * Mega;
};

/* RateLimiter usage:
 *
 *  auto limit = RateLimiter();
 *  timestamp_t period_ts = cfg.period * us;
 *  timestamp_t max_overshoot_ts = cfg.max_overshoot * ms;
 *  timestamp_t duration_ts = cfg.duration * s;
 *
 *  if (now > deadline + max_overshoot_ts) {
 *   deadline = now + period_ts;
 *  } else {
 *    while (now < deadline) {
 *      now = limit.gettime();
 *    }
 *    now = limit.gettime();
 *    deadline += period_ts;
 *  }
 *
 */
class RateLimiter {
public:
  RateLimiter() {}
  timestamp_t gettime() {
    ::timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    return timestamp_t(ts.tv_sec) * s + timestamp_t(ts.tv_nsec) * ns;
  }
};

template <class DT> class Timer {
public:
  Timer() : beg_(clock_::now()) {}

  void reset() { beg_ = clock_::now(); }

  double elapsed() const { return std::chrono::duration_cast<DT>(clock_::now() - beg_).count(); }

private:
  typedef std::chrono::high_resolution_clock clock_;
  // typedef std::chrono::duration<double, std::ratio<1> > second_;
  std::chrono::time_point<clock_> beg_;
};

/*
 * setThreadName
 * */
inline void setThreadName(std::thread &thread, const char *name, uint32_t tid) {
  char tname[16];
  snprintf(tname, 16, "daq-%s-%d", name, tid);
  auto handle = thread.native_handle();
  pthread_setname_np(handle, tname);
}

/*
 * getThreadName
 * */
inline void getThreadName(std::thread &thread, const char *name) {
  (void)name;

  char tname[16];
  (void)tname;
  // snprintf(tname, 16, "daq-%s-%d", name, tid);
  auto handle = thread.native_handle();
  (void)handle;
  // pthread_getname_np(handle, tname);
}

/*
 * getTime
 * */
inline timestamp_t getTime() {
  ::timespec ts;
  ::clock_gettime(CLOCK_MONOTONIC, &ts);
  return timestamp_t(ts.tv_sec) * s + timestamp_t(ts.tv_nsec) * ns;
}

/*
 * getExecutablePath
 * */
inline std::string getExecutablePath() {
  char exePath[PATH_MAX];
  ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath));
  if (len == -1 || len == static_cast<ssize_t>(sizeof(exePath)))
    len = 0;
  exePath[len] = '\0';
  return std::string(exePath);
}

inline std::string methodName(const std::string &prettyFunction) {
  size_t colons = prettyFunction.find("::");
  size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
  size_t end = prettyFunction.rfind("(") - begin;
  return prettyFunction.substr(begin, end) + "()";
}

inline std::string className(const std::string &prettyFunction) {
  size_t colons = prettyFunction.find("::");
  if (colons == std::string::npos)
    return "::";
  size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
  size_t end = colons - begin;

  return prettyFunction.substr(begin, end);
}

} // namespace utilities
} // namespace daqling

#endif // DAQLING_UTILITIES_COMMON_HPP
