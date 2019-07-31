#ifndef DAQLING_MODULE_NAME
    #error "This header should only be automatically included during the build with CMake"
#endif

#include "Utils/Logging.hpp"

#include DAQLING_MODULE_HEADER

namespace daqling::core {
    extern "C" {
        // declare to satisfy -Werror=missing-declarations
        DAQProcess* daqling_module_generator();

        // and then define
        DAQProcess* daqling_module_generator()
        {
            auto module = new DAQLING_MODULE_NAME();
            return static_cast<DAQProcess*>(module);
        }
    }
}
