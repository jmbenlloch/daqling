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
/// \endcond

#include "Common/DataFormat.hpp"
#include "EventBuilderModule.hpp"

using namespace std::chrono_literals;

EventBuilderModule::EventBuilderModule() {
  DEBUG("With config: " << m_config.dump() << " getState: " << this->getState());
  m_number_of_channels = m_config.getConnections()["receivers"].size();
}

EventBuilderModule::~EventBuilderModule() {}

void EventBuilderModule::start(unsigned run_num) {
  DAQProcess::start(run_num);
  DEBUG("getState: " << getState());
}

void EventBuilderModule::stop() {
  DAQProcess::stop();
  DEBUG("getState: " << this->getState());
}

void EventBuilderModule::runner() {
  DEBUG("Running...");
  while (m_run) {
    daqling::utilities::Binary b0, b1;
    while (!m_connections.get(0, b0) && m_run) {
      std::this_thread::sleep_for(10ms);
    }
    while (!m_connections.get(1, b1) && m_run) {
      std::this_thread::sleep_for(10ms);
    }

    b0 += b1;
    INFO("Size of build event: " << b0.size());
    m_connections.put(2, b0);
  }
  DEBUG("Runner stopped");
}
