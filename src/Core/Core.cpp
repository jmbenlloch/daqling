/**
 * Copyright (C) 2019-2021 CERN
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

#include <chrono>
#include <iomanip>

#include "Core.hpp"

using namespace daqling::core;
using namespace std::chrono_literals;

void Core::setupCommandServer() {
  ERS_INFO("Starting XML-RPC Server on port: " << m_port);
  m_command.setupServer(m_port);
}

bool Core::getShouldStop() { return m_command.getShouldStop(); }
