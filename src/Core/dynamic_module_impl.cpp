#ifndef DAQLING_MODULE_NAME
    #error "This header should only be automatically included during the build with CMake"
#endif

#include DAQLING_MODULE_HEADER

namespace daqling::core {
    extern "C" {
        // declare to satisfy -Werror=missing-declarations
        DAQProcess* daqling_module_create();
        void daqling_module_delete(DAQProcess*);

        // and then define
        DAQProcess* daqling_module_create()
        {
            auto module = new DAQLING_MODULE_NAME();
            return static_cast<DAQProcess*>(module);
        }

        void daqling_module_delete(DAQProcess *module)
        {
            delete static_cast<DAQLING_MODULE_NAME*>(module);
        }
    }
}
