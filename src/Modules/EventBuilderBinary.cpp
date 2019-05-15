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

// enrico.gamberini@cern.ch

/// \cond
#include <chrono>
/// \endcond

#include "Modules/EventBuilderBinary.hpp"

#define __METHOD_NAME__ daqling::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daqling::utilities::className(__PRETTY_FUNCTION__)

using namespace std::chrono_literals;

extern "C" EventBuilder *create_object() { return new EventBuilder; }

extern "C" void destroy_object(EventBuilder *object) { delete object; }

EventBuilder::EventBuilder() {
  INFO(__METHOD_NAME__ << " With config: " << m_config.dump() << " getState: " << this->getState());
}

EventBuilder::~EventBuilder() { INFO(__METHOD_NAME__); }

void EventBuilder::start() {
  DAQProcess::start();
  INFO(__METHOD_NAME__ << " getState: " << getState());
}

void EventBuilder::stop() {
  DAQProcess::stop();
  INFO(__METHOD_NAME__ << " getState: " << this->getState());
}

void EventBuilder::runner() {
  INFO(__METHOD_NAME__ << " Running...");
  while (m_run) {
    daqling::utilities::Binary b1, b2;
    while(!m_connections.get(1, b1) && m_run) {
      std::this_thread::sleep_for(10ms);
    }
    while(!m_connections.get(2, b2) && m_run) {
      std::this_thread::sleep_for(10ms);
    }

    daqling::utilities::Binary b3(b1);
    b3 += b2;
    INFO(__METHOD_NAME__ << " Size of build event: " << b3.size());
    m_connections.put(3, b3);
  }
  INFO(__METHOD_NAME__ << " Runner stopped");
}
