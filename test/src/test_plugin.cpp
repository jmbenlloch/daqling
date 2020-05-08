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
#include <chrono>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <thread>
/// \endcond

#include "Core/DAQProcess.hpp"
#include "Utils/Logging.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std::chrono_literals;
using logger = daqling::utilities::Logger;

using CreateFunc = daqling::core::DAQProcess *(void);
using DestroyFunc = void(daqling::core::DAQProcess *);

int main(int argc, char **argv) {
  auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("my_logger", sink);
  logger::set_instance(logger);

  if (argc == 1) {
    ERROR("No plugin name entered");
    return 1;
  }
  INFO("Loading " << argv[1]);
  std::string pluginName = "lib/lib" + std::string(argv[1]) + ".so";
  void *handle = dlopen(pluginName.c_str(), RTLD_NOW);
  if (handle == nullptr) {
    ERROR("Plugin name not valid");
    return 1;
  }

  auto create = reinterpret_cast<CreateFunc *>(dlsym(handle, "daqling_module_create"));
  auto destroy = reinterpret_cast<DestroyFunc *>(dlsym(handle, "daqling_module_create"));

  auto *dp = create();

  destroy(dp);
}
