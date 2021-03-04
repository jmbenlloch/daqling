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

#include "Core/Core.hpp"

#include "Utils/Ers.hpp"

using namespace std::chrono_literals;

int main(int argc, char **argv) {
  if (argc < 5) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::cerr << "Usage: " << argv[0]
              << " <name> <command-port> <core-log-level> <module-log-level>\n";
    return EXIT_FAILURE;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  std::string name = argv[1];
  // assign log-levels from arguments
  std::vector<std::string> const table = {"DEBUG", "LOG", "INFO", "WARNING", "ERROR"};
  std::vector<std::string>::const_iterator coreLvl;
  std::vector<std::string>::const_iterator moduleLvl;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  auto core_tup = std::make_tuple(&coreLvl, std::string(argv[3]), "INFO", "Core");
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  auto module_tup = std::make_tuple(&moduleLvl, std::string(argv[4]), "DEBUG", "Module");

  for (const auto & [ lvl, supplied_lvl, default_lvl, name ] : {core_tup, module_tup}) {
    if ((*lvl = find(table.begin(), table.end(), supplied_lvl)) == table.end()) {
      *lvl = find(table.begin(), table.end(), default_lvl);
      ERS_WARNING("Unknown loglevel '" << supplied_lvl << "', defaulting to level '" << default_lvl
                                       << "' for '" << name << "' logger");
    }
  }

  // helper variables - stream setup:
  bool CoreHasLowestLvl = coreLvl < moduleLvl;
  auto lowestLvl = (CoreHasLowestLvl ? coreLvl : moduleLvl);
  auto highestLvl = (CoreHasLowestLvl ? moduleLvl : coreLvl);
  std::string filter = (CoreHasLowestLvl ? "filter(core)," : "filter(module),");
  // set up the streams:
  for (auto level = table.begin(); level != table.end(); ++level) {
    std::string stream = (static_cast<std::string>("TDAQ_ERS_")) + (*level);
    std::string configs = ""; // NOLINT  sets configs=configs for some reason
    if (level < lowestLvl) {
      configs += "null";
    } else {
      if (level >= lowestLvl && level < highestLvl) {
        configs += filter;
      }
      if (*level == "WARNING" || *level == "ERROR") {
        configs += "throttle,dlstderr";
      } else {
        configs += "dlstdout";
      }
      configs += ",ZMQSink(" + name + ")";
    }
    setenv(stream.c_str(), configs.c_str(), 0);
  }
  // Setup fatal stream to terminate
  setenv("TDAQ_ERS_FATAL", "dlstderr,exit", 0);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  unsigned port = strtoul(argv[2], nullptr, 0);
  daqling::core::Core c(port);

  c.setupCommandServer();

  std::mutex *mtx = c.getMutex();
  std::condition_variable *cv = c.getCondVar();
  std::unique_lock<std::mutex> lk(*mtx);

  cv->wait(lk, [&] { return c.getShouldStop(); });
  lk.unlock();

  return EXIT_SUCCESS;
}
