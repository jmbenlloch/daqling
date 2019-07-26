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

/// \cond
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
/// \endcond

#include "Modules/BoardReader.hpp"
#include "Modules/EventBuilder.hpp"
#include "Utilities/Logging.hpp"
#include "Core/ConnectionManager.hpp"

using namespace std::chrono_literals;

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

    daqling::core::DAQProcess *(*create)(...);
    void (*destroy)(daqling::core::DAQProcess *);

    create = (daqling::core::DAQProcess * (*)(...)) dlsym(handle, "create_object");
    destroy = (void (*)(daqling::core::DAQProcess *))dlsym(handle, "destroy_object");

    std::string name = "hello";
    int num = 42;
    
    auto *dp = (daqling::core::DAQProcess *)create(name, num);

    dp->start();
    std::this_thread::sleep_for(2s);
    dp->stop();
    destroy(dp);
}
