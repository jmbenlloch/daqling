#ifndef DAQ_UTILITIES_LOGGING_H
#define DAQ_UTILITIES_LOGGING_H

#include <memory>
#include <sstream>

#include "spdlog/spdlog.h"

namespace daq {
namespace utilities
{
	class logging
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
}
}

#undef INFO

#define DEBUG(MSG) do { std::ostringstream writer; writer << MSG; felix::base::Logger::instance()->debug(writer.str()); } while (0)
#define INFO(MSG) do { std::ostringstream writer; writer << MSG; felix::base::Logger::instance()->info(writer.str()); } while (0)
#define NOTICE(MSG) do { std::ostringstream writer; writer << MSG; felix::base::Logger::instance()->notice(writer.str()); } while (0)
#define WARNING(MSG) do { std::ostringstream writer; writer << MSG; felix::base::Logger::instance()->warn(writer.str()); } while (0)
#define ERROR(MSG) do { std::ostringstream writer; writer << MSG; felix::base::Logger::instance()->error(writer.str()); } while (0)
#define CRITICAL(MSG) do { std::ostringstream writer; writer << MSG; felix::base::Logger::instance()->critical(writer.str()); } while (0)
#define ALERT(MSG) do { std::ostringstream writer; writer << MSG; felix::base::Logger::instance()->alert(writer.str()); } while (0)

#endif
