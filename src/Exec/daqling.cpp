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

using namespace std::chrono_literals;

std::shared_ptr<spdlog::logger> daqling::utilities::Logger::m_logger;
std::shared_ptr<spdlog::logger> daqling::utilities::Logger::m_module_logger;

int main(int argc, char **argv) {
  auto root_logger = spdlog::stdout_logger_mt("root");
  root_logger->set_pattern("[%Y-%m-%d %T.%e] [%n] [%l] [%t] [%@] %v");

  auto core_logger = root_logger->clone("core");
  auto module_logger = root_logger->clone("module");

  core_logger->set_level(spdlog::level::level_enum::critical);
  module_logger->set_level(spdlog::level::level_enum::debug);

  daqling::utilities::Logger::set_instance(core_logger);
  daqling::utilities::Logger::m_module_logger = module_logger;

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
