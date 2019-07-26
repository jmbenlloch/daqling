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

#include "Modules/BoardReader.hpp"


using namespace std::chrono_literals;
using namespace std::chrono;

extern "C" BoardReader *create_object(std::string name, int num) {
  return new BoardReader(name, num);
}

extern "C" void destroy_object(BoardReader *object) { delete object; }

BoardReader::BoardReader(std::string name, int num) {
  INFO("Passed " << name << " " << num << " with constructor");
  INFO("With config: " << m_config.dump());
}

BoardReader::~BoardReader() {}

void BoardReader::start() {
  DAQProcess::start();
  INFO("getState: " << this->getState());
}

void BoardReader::stop() {
  DAQProcess::stop();
  INFO("getState: " << this->getState());
}

void BoardReader::runner() {
  INFO("Running...");
  while (m_run) {
    m_connections.putStr(1, "WoofBla");

    std::this_thread::sleep_for(10ms);
  }
  INFO("Runner stopped");
}
