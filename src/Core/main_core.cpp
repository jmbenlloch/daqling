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
#include "Utilities/Logging.hpp"

using namespace std::chrono_literals;

int main(int argc, char **argv) {
  if (argc == 1) {
    ERROR("No command port provided!");
    return 1;
  }

  if (argc == 3 && (std::string)argv[2] == "debug") {
    daq::utilities::set_log_level("debug");
    INFO("Log level DEBUG");
  } else {
    daq::utilities::set_log_level("info");
    INFO("Log level INFO");
  }

  int port = atoi(argv[1]);
  DEBUG("Port " << port);
  daq::core::Core c(port, "tcp", "*");

  c.setupCommandPath();
  c.startCommandHandler();

  std::mutex *mtx = c.getMutex();
  std::condition_variable *cv = c.getCondVar();

  std::unique_lock<std::mutex> lk(*mtx);

  cv->wait(lk, [&] { return c.getShouldStop(); });
  lk.unlock();
  std::this_thread::sleep_for(100ms);  // allow time for command handler to stop

  return 0;
}
