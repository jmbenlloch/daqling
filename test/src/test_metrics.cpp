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

#include "Core/Statistics.hpp"
#include "zmq.hpp"
#include <atomic>
#include <unistd.h>

int main(int, char **) {

  std::atomic<float> buffer_occupation;
  std::atomic<int> packets;
  buffer_occupation = 0.1;
  packets = 0;

  zmq::context_t context(1);
  std::unique_ptr<zmq::socket_t> publisher;
  publisher = std::make_unique<zmq::socket_t>(context, ZMQ_PUB);
  publisher->bind("tcp://*:5556");

  daqling::core::Statistics stat(std::ref(publisher));
  stat.start();

  stat.registerVariable<std::atomic<float>, float>(&buffer_occupation, "AverageBufferOccupation",
                                                   daqling::core::metrics::AVERAGE,
                                                   daqling::core::metrics::FLOAT, 2);
  stat.registerVariable<std::atomic<float>, float>(&buffer_occupation, "BufferOccupation",
                                                   daqling::core::metrics::LAST_VALUE,
                                                   daqling::core::metrics::FLOAT);
  stat.registerVariable<std::atomic<int>, int>(
      &packets, "PacketsRate", daqling::core::metrics::RATE, daqling::core::metrics::INT);
  // stat.registerVariable("NumberOfPackets", &packets);
  while (1) {
    usleep(500000);
    // std::cout<<"bla"<<std::endl;
    packets += 2;
    buffer_occupation = buffer_occupation + 3.1;
    std::cout << "Buffer occupation: " << buffer_occupation << std::endl;
  }

  return 0;
}
