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

#include <string>

#include "Core/DAQProcess.hpp"

class ReadoutInterfaceModule : public daqling::core::DAQProcess {
  void pause();
  void resume();

public:
  ReadoutInterfaceModule();
  ~ReadoutInterfaceModule();
  void start(unsigned run_num);
  void stop();

  void runner();

private:
  unsigned m_board_id;
  std::chrono::microseconds m_delay_us;
  size_t m_min_payload, m_max_payload;
  bool m_pause;
};
