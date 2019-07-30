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

#include "EventBuilder.hpp"


using namespace std::chrono_literals;

extern "C" EventBuilder *create_object() { return new EventBuilder; }

extern "C" void destroy_object(EventBuilder *object) { delete object; }

EventBuilder::EventBuilder() {
  INFO("With config: " << m_config.dump() << " getState: " << this->getState());
}

EventBuilder::~EventBuilder() {}

void EventBuilder::start() {
  DAQProcess::start();
  INFO("getState: " << getState());
}

void EventBuilder::stop() {
  DAQProcess::stop();
  INFO("getState: " << this->getState());
}

void EventBuilder::runner() {
  INFO("Running...");
  const unsigned c_packing = 20;
  while (m_run) {
    std::string packed = "";
    for (unsigned i = 0; i < c_packing;) {
      std::string s1{m_connections.getStr(1)};
      std::string s2{m_connections.getStr(2)};
      if (s1 != "") {
        INFO("Received on channel 1 " << s1);
        packed += s1;
        i++;
      }
      if (s2 != "") {
        INFO("Received on channel 2 " << s2);
        packed += s2;
        i++;
      }
    }
    if (packed != "") {
      INFO("Sending mega string on channel 3 " << packed);
      m_connections.putStr(3, packed);
    }
  }
  INFO("Runner stopped");
}
