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
#include "Utils/Logging.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std::chrono_literals;
using logger = daqling::utilities::Logger;

int main(int argc, char **argv) {
  auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto core_logger = std::make_shared<spdlog::logger>("core", stdout_sink);
  auto module_logger = std::make_shared<spdlog::logger>("module", stdout_sink);

  for (auto &logger : {&*core_logger, &*module_logger}) {
    logger->set_pattern("[%Y-%m-%d %T.%e] [%n] [%l] [%t] [%@] %v");
  }

  core_logger->set_level(spdlog::level::info);
  module_logger->set_level(spdlog::level::debug);

  logger::set_instance(std::move(core_logger));
  logger::set_module_instance(std::move(module_logger));

  if (argc == 1) {
    ERROR("No command port provided!");
    return 1;
  }

  int port = atoi(argv[1]);
  daqling::core::Core c(port, "tcp", "*");

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
