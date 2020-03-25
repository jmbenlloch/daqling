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

/**
 * @file Logging.hpp
 *
 * Static logging implementation exposing macros to log to different levels.
 * `set_instance` must be called before any log entries are made.
 * Otherwise is UB.
 */

#ifndef DAQLING_UTILITIES_LOGGING_HPP
#define DAQLING_UTILITIES_LOGGING_HPP

/// \cond
#include <cassert>
#include <memory>
#include <sstream>
/// \endcond

#include "Common.hpp"
#include "Utils/zhelpers.hpp"
#include "hedley.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/spdlog.h"

#undef SPDLOG_FUNCTION
#define SPDLOG_FUNCTION __PRETTY_FUNCTION__

#undef INFO

#define __METHOD_NAME__ daqling::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daqling::utilities::className(__PRETTY_FUNCTION__)

#define LOG(LEVEL, MSG)                                                                            \
  do {                                                                                             \
    std::ostringstream writer;                                                                     \
    writer << MSG;                                                                                 \
    SPDLOG_LOGGER_CALL(daqling::utilities::Logger::instance(), LEVEL, writer.str());               \
  } while (0)

#define TRACE(MSG) LOG(spdlog::level::trace, MSG)
#define DEBUG(MSG) LOG(spdlog::level::debug, MSG)
#define INFO(MSG) LOG(spdlog::level::info, MSG)
#define WARNING(MSG) LOG(spdlog::level::warn, MSG)
#define ERROR(MSG) LOG(spdlog::level::err, MSG)
#define CRITICAL(MSG) LOG(spdlog::level::critical, MSG)

// Level aliases
#define NOTICE(MSG) LOG(spdlog::level::info, MSG)
#define ALERT(MSG) LOG(spdlog::level::warn, MSG)

namespace daqling::utilities {
using LoggerType = std::shared_ptr<spdlog::logger>;

class Logger {
private:
  static bool m_module_logger_set; // may only be set once
  static LoggerType m_module_logger;
  static LoggerType m_logger;

public:
  /**
   * Sets the current logger instance.
   *
   * @warning May only be called if no logger is already set.
   * @todo Replace assertion with an exception throw
   */
  HEDLEY_PRIVATE
  static void set_instance(LoggerType logger) {
    assert(!m_logger);
    m_logger = logger;
  }

  /**
   * Gets the logger instance.
   *
   * @warning May only be called if a logger is already set.
   * @todo Replace assertion with an exception throw
   */
  HEDLEY_PRIVATE
  static LoggerType instance() {
    assert(m_logger);
    return m_logger;
  }

  /**
   * Sets the logger assigned to the dynamically loaded module.
   *
   * @warning May only be called once.
   * @todo Replace assertion with an exception throw
   */
  static void set_module_instance(LoggerType logger) {
    assert(!std::exchange(m_module_logger_set, true));
    m_module_logger = logger;
  }

  /**
   * Gets the logger assigned to the dynamically loaded module.
   *
   * @warning May only be called if a module logger is already set.
   * @todo Replace assertion with an exception throw
   */
  static LoggerType get_module_instance() {
    assert(m_module_logger);
    return m_module_logger;
  }
};

template <typename Mutex> class zmq_sink : public spdlog::sinks::base_sink<Mutex> {
public:
  zmq_sink(std::string name) : m_name(name) {
    m_context = std::make_unique<zmq::context_t>(1);
    m_socket = std::make_unique<zmq::socket_t>(*(m_context.get()), ZMQ_PUB);
    m_socket->connect("tcp://localhost:6542");
  }

private:
  std::unique_ptr<zmq::context_t> m_context;
  std::unique_ptr<zmq::socket_t> m_socket;
  std::string m_name;

protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    fmt::memory_buffer formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

    zmq::message_t topic(m_name.size());
    memcpy(topic.data(), m_name.data(), m_name.size());
    m_socket->send(topic, ZMQ_SNDMORE);

    std::string str = fmt::to_string(formatted);
    zmq::message_t message(str.size());
    memcpy(message.data(), str.data(), str.size());
    m_socket->send(message);
  }

  void flush_() override {}
};

using zmq_sink_mt = zmq_sink<std::mutex>;

static std::string sink_pattern() {
  std::ostringstream pattern;
  pattern << "[%Y-%m-%d %T.%e] [%n] [%l] [%@] [%!]  %v";
  return pattern.str();
}

/**
 * Sets the log level of the logger instance.
 */
inline void set_log_level(spdlog::level::level_enum level) { Logger::instance()->set_level(level); }

typedef spdlog::level::level_enum level;
} // namespace daqling::utilities

#endif // DAQLING_UTILITIES_LOGGING_HPP
