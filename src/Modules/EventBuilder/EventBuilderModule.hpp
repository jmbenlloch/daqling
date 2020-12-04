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

#pragma once

#include "Core/DAQProcess.hpp"

class EventBuilderModule : public daqling::core::DAQProcess {

  unsigned m_nreceivers;
  std::atomic<size_t> m_eventmap_size;
  std::atomic<size_t> m_complete_ev_size_guess;

public:
  EventBuilderModule();
  ~EventBuilderModule();

  void configure();
  void start(unsigned run_num);
  void stop();

  void runner() noexcept;
};
