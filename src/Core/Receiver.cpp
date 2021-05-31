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

#include "Receiver.hpp"
#include "Utils/Ers.hpp"
using namespace daqling::core;
Receiver::Receiver(uint chid) : m_chid(chid) {
  m_msg_handled.store(0);
  m_pcq_size.store(0);
}

bool Receiver::start() {
  ERS_DEBUG(0, "Default start method.");
  return true;
}
bool Receiver::stop() {
  ERS_DEBUG(0, "Default stop method.");
  return true;
}
bool Receiver::sleep_receive(DataType &bin) {
  if (receive(bin)) {
    return true;
  }
  std::this_thread::sleep_for((std::chrono::milliseconds(m_sleep_duration)));
  return false;
}
void Receiver::set_sleep_duration(uint ms) { m_sleep_duration = ms; }
std::atomic<size_t> &Receiver::getMsgsHandled() { return m_msg_handled; }
std::atomic<size_t> &Receiver::getPcqSize() { return m_pcq_size; }
