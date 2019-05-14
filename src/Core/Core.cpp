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
#include <iomanip>
/// \endcond

#include "Core/Core.hpp"

#define __METHOD_NAME__ daq::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daq::utilities::className(__PRETTY_FUNCTION__)

using namespace daq::core;
using namespace std::chrono_literals;

bool Core::setupCommandPath() {
  std::string connStr(m_protocol + "://" + m_address + ":" + std::to_string(m_port));
  INFO(" BINDING COMMAND SOCKET : " << connStr);
  bool rv = m_connections.setupCommandConnection(1, connStr);
  return rv;
}

bool Core::startCommandHandler() {
  m_command.startCommandHandler();
  return true;
}

bool Core::getShouldStop() { return m_command.getShouldStop(); }
