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

#include "FollyProducerConsumer.hpp"
#include "Utils/ConnectionMacros.hpp"

using namespace daqling::queue;
REGISTER_QUEUE(FollyProducerConsumer)
template <typename T>
FollyProducerConsumer<T>::FollyProducerConsumer(const nlohmann::json &j)
    : m_queue(j.at("queue_size").get<unsigned int>()) {}
template <typename T> bool FollyProducerConsumer<T>::read(DataType &bin) {
  return m_queue.read(static_cast<T &>(bin));
}
template <typename T> bool FollyProducerConsumer<T>::write(DataType &bin) {
  return m_queue.write(std::move(static_cast<T &>(bin)));
}
template <typename T> uint FollyProducerConsumer<T>::sizeGuess() { return m_queue.sizeGuess(); }
template <typename T> uint FollyProducerConsumer<T>::capacity() { return m_queue.capacity(); }