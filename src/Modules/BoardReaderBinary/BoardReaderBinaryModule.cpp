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
#include <iomanip>
#include <random>
/// \endcond

#include "BoardReaderBinaryModule.hpp"


using namespace std::chrono_literals;
using namespace std::chrono;

struct header_t {
  uint16_t payload_size;
  uint16_t source_id;
  uint32_t seq_number;
  uint64_t timestamp;
} __attribute__((__packed__));

struct data_t {
  header_t header;
  char payload[24000];
} __attribute__((__packed__));

BoardReaderBinaryModule::BoardReaderBinaryModule() {
  /* INFO("Passed " << name << " " << num << " with constructor"); */
  INFO("With config: " << m_config.dump());

  m_board_id = m_config.getConfig()["settings"]["board_id"];
}

BoardReaderBinaryModule::~BoardReaderBinaryModule() {}

void BoardReaderBinaryModule::start() {
  DAQProcess::start();
  INFO("getState: " << this->getState());
}

void BoardReaderBinaryModule::stop() {
  DAQProcess::stop();
  INFO("getState: " << this->getState());
}

void BoardReaderBinaryModule::runner() {
  unsigned sequence_number = 0;
  microseconds timestamp;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(200, 1500);

  INFO("Running...");
  while (m_run) {
    timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch());
    const unsigned payload_size = static_cast<unsigned>(dis(gen));
    const unsigned total_size = sizeof(header_t) + sizeof(char) * payload_size;

    INFO("sequence number " << sequence_number << "  >>  timestamp " << std::hex
                         << "0x" << timestamp.count() << std::dec << "  >>  payload size "
                         << payload_size);

    std::unique_ptr<data_t> data(static_cast<data_t *>(malloc(total_size)));
    data->header.payload_size = payload_size;
    data->header.seq_number = sequence_number;
    data->header.source_id = m_board_id;
    data->header.timestamp = static_cast<uint64_t>(timestamp.count());
    memset(data->payload, 0xFE, payload_size);

    // ready to be sent to EB
    auto binary = daqling::utilities::Binary(static_cast<const void *>(data.get()), total_size);

    // print binary
    // INFO("\n" << binary);

    m_connections.put(1, binary);

    sequence_number++;
    std::this_thread::sleep_for(500ms);
  }
  INFO("Runner stopped");
}
