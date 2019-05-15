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

#ifndef DAQLING_MODULES_EVENTBUILDER_HPP
#define DAQLING_MODULES_EVENTBUILDER_HPP

#include "Core/DAQProcess.hpp"

class EventBuilder : public daqling::core::DAQProcess {
 public:
  EventBuilder();
  ~EventBuilder();

  void start();
  void stop();

  void runner();
};

#endif // DAQLING_MODULES_EVENTBUILDER_HPP
