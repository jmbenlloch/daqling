#ifndef DAQLING_MODULE_NAME
    #error "This header should only be automatically included during the build with CMake"
#endif

#include "Utils/Logging.hpp"
#include DAQLING_MODULE_HEADER

// TODO: explain
__attribute__((visibility("hidden")))
std::shared_ptr<spdlog::logger> daqling::utilities::Logger::m_logger;

namespace daqling::core {
    extern "C" {
        // declare to satisfy -Werror=missing-declarations
        DAQProcess* daqling_module_create(std::shared_ptr<spdlog::logger>);
        void daqling_module_delete(DAQProcess*);
    }

    // and then define
    DAQProcess* daqling_module_create(std::shared_ptr<spdlog::logger> logger)
    {
        // must be set first so that we get "[module]" during setup
        // TODO: improve docs
        const std::string logger_name = logger->name();
        daqling::utilities::Logger::set_instance(logger);

        auto module = new DAQLING_MODULE_NAME();
        return static_cast<DAQProcess*>(module);
    }

    void daqling_module_delete(DAQProcess *module)
    {
        delete static_cast<DAQLING_MODULE_NAME*>(module);
    }
}
