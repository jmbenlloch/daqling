#ifndef DAQLING_EXECUTABLE_NAME
#error "This header should only be automatically included during the build with CMake"
#endif

#include "Utils/Logging.hpp"

/*
 * Defines required logging fields.
 */
daqling::utilities::LoggerType daqling::utilities::Logger::m_logger;
daqling::utilities::LoggerType daqling::utilities::Logger::m_module_logger;
bool daqling::utilities::Logger::m_module_logger_set;
