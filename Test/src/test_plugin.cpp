// enrico.gamberini@cern.ch

#include <dlfcn.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "modules/BoardReader.hpp"
#include "modules/EventBuilder.hpp"
#include "utilities/logging.hpp"

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        ERROR("No plugin name entered");
        return 1;
    }
    INFO("Loading " << argv[1]);
    std::string pluginName = "lib" + (std::string)argv[1] + ".so";
    void *handle = dlopen(pluginName.c_str(), RTLD_LAZY);
    if (handle == 0)
    {
        ERROR("Plugin name not valid");
        return 1;
    }

    BaseClass *(*create)();
    void (*destroy)(BaseClass *);

    create = (BaseClass * (*)()) dlsym(handle, "create_object");
    destroy = (void (*)(BaseClass *))dlsym(handle, "destroy_object");

    BaseClass *bc = (BaseClass *)create();

    bc->start();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    bc->stop();
    destroy(bc);
}
