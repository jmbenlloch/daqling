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
/// \endcond

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "Utilities/Common.hpp"

namespace daqling {
namespace utilities
{
	class Logger
	{
		static std::shared_ptr<spdlog::logger> logger;

	public:
		static void init_console()
		{
		  logger = spdlog::stdout_logger_mt("console");
		  setup();
		}

		static void init_file(std::string filename)
		{
			logger = spdlog::rotating_logger_mt("file_logger", filename, 1048576 * 5, 5);
		}

		static void setup()
		{
			logger->set_pattern("[%Y-%m-%d %T.%e] [%l] [%t] %v");
		}

		static std::shared_ptr<spdlog::logger> instance()
		{
				if(!logger)
				init_console(); // log to console by default
			return logger;
		}
	};

	inline void set_log_level(spdlog::level::level_enum level)
	{
		Logger::instance()->set_level(level);
	}

	inline void set_log_level(std::string level)
	{
		if(level == "off") {
			Logger::instance()->set_level(spdlog::level::off);
		} else if(level == "debug") {
			Logger::instance()->set_level(spdlog::level::debug);
		} else if(level == "info") {
			Logger::instance()->set_level(spdlog::level::info);
		} else if(level == "notice") {
			Logger::instance()->set_level(spdlog::level::info);
		} else if(level == "warn") {
			Logger::instance()->set_level(spdlog::level::warn);
		} else if(level == "err") {
			Logger::instance()->set_level(spdlog::level::err);
		} else if(level == "critical") {
			Logger::instance()->set_level(spdlog::level::critical);
		} else if(level == "alert") {
			Logger::instance()->set_level(spdlog::level::warn);
		}
	}

typedef spdlog::level::level_enum level;
} // namespace utilities
} // namespace daqling

#undef INFO

#define __METHOD_NAME__ daqling::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daqling::utilities::className(__PRETTY_FUNCTION__)

#define DEBUG(MSG)    do { std::ostringstream writer; writer << "[" << __METHOD_NAME__ << "] " << MSG; daqling::utilities::Logger::instance()->debug(writer.str()); } while (0)
#define INFO(MSG)     do { std::ostringstream writer; writer << "[" << __METHOD_NAME__ << "] " << MSG; daqling::utilities::Logger::instance()->info(writer.str()); } while (0)
#define NOTICE(MSG)   do { std::ostringstream writer; writer << "[" << __METHOD_NAME__ << "] " << MSG; daqling::utilities::Logger::instance()->notice(writer.str()); } while (0)
#define WARNING(MSG)  do { std::ostringstream writer; writer << "[" << __METHOD_NAME__ << "] " << MSG; daqling::utilities::Logger::instance()->warn(writer.str()); } while (0)
#define ERROR(MSG)    do { std::ostringstream writer; writer << "[" << __METHOD_NAME__ << "] " << MSG; daqling::utilities::Logger::instance()->error(writer.str()); } while (0)
#define CRITICAL(MSG) do { std::ostringstream writer; writer << "[" << __METHOD_NAME__ << "] " << MSG; daqling::utilities::Logger::instance()->critical(writer.str()); } while (0)
#define ALERT(MSG)    do { std::ostringstream writer; writer << "[" << __METHOD_NAME__ << "] " << MSG; daqling::utilities::Logger::instance()->alert(writer.str()); } while (0)

#endif // DAQLING_UTILITIES_LOGGING_HPP
