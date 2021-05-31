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

#include "MoodyReaderWriter.hpp"
#include "Utils/ConnectionMacros.hpp"

using namespace daqling::queue;
REGISTER_QUEUE(MoodyReaderWriter)
template <typename T>
MoodyReaderWriter<T>::MoodyReaderWriter(const nlohmann::json &j)
    : m_queue(j.at("queue_size").get<unsigned int>()) {
  m_capacity = j.at("queue_size").get<unsigned int>();
}
template <typename T> bool MoodyReaderWriter<T>::read(DataType &bin) {
  return m_queue.try_dequeue(static_cast<T &>(bin));
}
// bool MoodyReaderWriter::sleep_read(daqling::utilities::Binary& bin)
// {
//     return m_queue.wait_dequeue_timed(bin,std::chrono::milliseconds(m_sleep_duration));
// }
template <typename T> bool MoodyReaderWriter<T>::write(DataType &bin) {
  // return m_queue.try_emplace(std::move(bin));
  return m_queue.try_enqueue(static_cast<T &>(bin));
}
template <typename T> uint MoodyReaderWriter<T>::sizeGuess() { return m_queue.size_approx(); }
template <typename T> uint MoodyReaderWriter<T>::capacity() { return m_capacity; }