#ifndef DAQLING_EXECUTABLE_NAME
    #error "This header should only be automatically included during the build with CMake"
#endif

#include "Utils/Logging.hpp"

std::shared_ptr<spdlog::logger> daqling::utilities::Logger::m_logger;
std::shared_ptr<spdlog::logger> daqling::utilities::Logger::m_module_logger;
