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

#include "Utils/Logging.hpp"
#include "Utils/zhelpers.hpp"
#include <chrono>
#include <thread>

struct data_t {
  uint8_t tag;
  uint32_t blab : 24;
  uint32_t whatever;
} __attribute__((__packed__));

int main(int argc, char *argv[]) {
  zmq::context_t context(1);
  zmq::socket_t publisher(context, ZMQ_PUB);

  if (argc == 2)
    publisher.bind(argv[1]);
  else
    publisher.bind("tcp://*:5556");

  //  Ensure subscriber connection has time to complete
  std::this_thread::sleep_for(std::chrono::seconds(1));

  data_t d;
  d.tag = 126;
  d.blab = 115453;
  d.whatever = 24;

  std::cout << "size of data_t " << sizeof(data_t) << std::endl;
  while (1) {
    zmq::message_t msg(sizeof(data_t));
    memcpy(msg.data(), &d, sizeof(data_t));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "-----------\nMsg size " << msg.size() << std::endl;
    std::cout << "-> sent " << publisher.send(msg) << std::endl;
    ++d.whatever;
  }
  return 0;
}
