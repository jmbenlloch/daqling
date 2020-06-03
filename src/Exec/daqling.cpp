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
#include <cstdlib>
/// \endcond

#include "Core/Core.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std::chrono_literals;
using logger = daqling::utilities::Logger;

int main(int argc, char **argv) {
  if (argc < 5) {
    std::cerr << "Usage: " << argv[0]
              << " <name> <command-port> <core-log-level> <module-log-level>\n";
    return EXIT_FAILURE;
  }

  std::string name = argv[1];
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
  sinks.push_back(std::make_shared<daqling::utilities::zmq_sink_mt>(name));

  auto core_logger = std::make_shared<spdlog::logger>("core", begin(sinks), end(sinks));
  auto module_logger = std::make_shared<spdlog::logger>("module", begin(sinks), end(sinks));

  // Set default sink pattern
  for (auto sink : sinks) {
    sink->set_pattern(daqling::utilities::sink_pattern());
  }

  // Assign the logger globals, allowing us to use logging macros.
  logger::set_instance(core_logger);
  logger::set_module_instance(module_logger);

  // Parse and set log level for both logers
  auto core_ctx = std::make_tuple(core_logger, std::string(argv[3]), spdlog::level::info);
  auto module_ctx = std::make_tuple(module_logger, std::string(argv[4]), spdlog::level::debug);
  for (auto[logger, supplied_lvl, default_lvl] : {core_ctx, module_ctx}) {
    std::transform(supplied_lvl.begin(), supplied_lvl.end(), supplied_lvl.begin(), ::tolower);

    if (auto lvl = spdlog::level::from_str(supplied_lvl);
        lvl == spdlog::level::off && supplied_lvl != "off") {
      // Supplied log level does not exist so spdlog returned the default level::off. Use our own
      // default instead.
      WARNING("Unknown loglevel '" << supplied_lvl << "', defaulting to level '"
                                   << spdlog::level::to_string_view(default_lvl).data() << "' for '"
                                   << logger->name() << "' logger");
      logger->set_level(default_lvl);
    } else {
      // Log level exists, set it.
      logger->set_level(lvl);
    }
  }

  unsigned port = strtoul(argv[2], NULL, 0);
  daqling::core::Core c(port);

  c.setupCommandServer();

  std::mutex *mtx = c.getMutex();
  std::condition_variable *cv = c.getCondVar();
  std::unique_lock<std::mutex> lk(*mtx);

  cv->wait(lk, [&] { return c.getShouldStop(); });
  lk.unlock();

  return EXIT_SUCCESS;
}
