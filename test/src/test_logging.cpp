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

#include "Core/ConnectionManager.hpp"
#include "Utils/Logging.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

using logger = daqling::utilities::Logger;

int main(int, char **) {
  auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("my_logger", sink);
  logger::set_instance(logger);

  INFO("WOOF WOOF");
  WARNING("Ugh!" << 12345 << "bof bof" << '\n');

  INFO("Testing ConnectionManager.hpp");
  std::ignore = daqling::core::ConnectionManager::instance();

  ERROR("About to die...");
  return 0;
}
