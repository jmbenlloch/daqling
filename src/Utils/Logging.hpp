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
 * @author https://gitlab.cern.ch/atlas-tdaq-felix/felixbase
 */

#ifndef DAQLING_UTILITIES_LOGGING_HPP
#define DAQLING_UTILITIES_LOGGING_HPP

/// \cond
#include <memory>
#include <sstream>
#include <cassert>
#include <memory>
/// \endcond

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "Common.hpp"

#undef SPDLOG_FUNCTION
#define SPDLOG_FUNCTION __PRETTY_FUNCTION__

#undef INFO

#define __METHOD_NAME__ daqling::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daqling::utilities::className(__PRETTY_FUNCTION__)

#define LOG(LEVEL, MSG) \
    do \
    { \
        std::ostringstream writer; \
        writer << MSG; \
        SPDLOG_LOGGER_CALL(daqling::utilities::Logger::instance(), LEVEL, writer.str()); \
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

	class Logger
	{
	private:
		static bool m_module_logger_set;
		static LoggerType m_module_logger;
		static LoggerType m_logger;

	public:

		__attribute__((visibility("hidden")))
		static void set_instance(LoggerType logger)
		{
			assert(!m_logger);
			m_logger = logger;
		}

		__attribute__((visibility("hidden")))
		static LoggerType instance()
		{
			assert(m_logger);
			return m_logger;
		}

        static void set_module_instance(LoggerType logger)
		{
			assert(!std::exchange(m_module_logger_set, true));
			m_module_logger = logger;
		}

		static LoggerType get_module_instance()
		{
			assert(m_module_logger);
			return m_module_logger;
		}
	};

	inline void set_log_level(spdlog::level::level_enum level)
	{
		Logger::instance()->set_level(level);
	}

typedef spdlog::level::level_enum level;
} // namespace daqling::utilities

#endif // DAQLING_UTILITIES_LOGGING_HPP
