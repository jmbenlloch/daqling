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
        SPDLOG_LOGGER_CALL(daqling::utilities::Logger::m_logger, LEVEL, writer.str()); \
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

namespace daqling {
namespace utilities
{
	class Logger
	{
	public:
		static std::shared_ptr<spdlog::logger> m_logger;

	/* public: */
		static std::shared_ptr<spdlog::logger> m_module_logger;

		static void set_instance(std::shared_ptr<spdlog::logger> logger)
		{
			assert(!m_logger);
			m_logger = logger;
		}

		static std::shared_ptr<spdlog::logger> instance()
		{
			assert(m_logger);
			return m_logger;
		}
	};

	inline void set_log_level(spdlog::level::level_enum level)
	{
		Logger::instance()->set_level(level);
	}

typedef spdlog::level::level_enum level;
} // namespace utilities
} // namespace daqling

#endif // DAQLING_UTILITIES_LOGGING_HPP
