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
#include <random>
/// \endcond

#include "Common/DataFormat.hpp"
#include "ReadoutInterfaceModule.hpp"

using namespace std::chrono_literals;
using namespace std::chrono;

ReadoutInterfaceModule::ReadoutInterfaceModule() {
  DEBUG("With config: " << m_config.dump());

  m_board_id = m_config.getSettings()["board_id"];
  m_delay_us = std::chrono::microseconds(m_config.getSettings()["delay_us"]);
  m_min_payload = m_config.getSettings()["payload"]["min"];
  m_max_payload = m_config.getSettings()["payload"]["max"];

  m_pause = false;
  registerCommand("pause", "paused", &ReadoutInterfaceModule::pause, this);
  registerCommand("resume", "running", &ReadoutInterfaceModule::resume, this);
}

ReadoutInterfaceModule::~ReadoutInterfaceModule() {}

void ReadoutInterfaceModule::pause() { m_pause = true; }

void ReadoutInterfaceModule::resume() { m_pause = false; }

void ReadoutInterfaceModule::start(unsigned run_num) {
  DAQProcess::start(run_num);
  DEBUG("getState: " << this->getState());
}

void ReadoutInterfaceModule::stop() {
  DAQProcess::stop();
  DEBUG("getState: " << this->getState());
}

void ReadoutInterfaceModule::runner() {
  unsigned sequence_number = 0;
  microseconds timestamp;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(static_cast<int>(m_min_payload),
                                      static_cast<int>(m_max_payload));

  DEBUG("Running...");
  while (m_run) {
    if (m_pause) {
      INFO("Paused at sequence number " << sequence_number);
      while (m_pause && m_run) {
        std::this_thread::sleep_for(10ms);
      }
    }
    timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch());
    const unsigned payload_size = static_cast<unsigned>(dis(gen));
    const unsigned total_size = sizeof(header_t) + sizeof(char) * payload_size;

    DEBUG("sequence number " << sequence_number << "  >>  timestamp " << std::hex << "0x"
                             << timestamp.count() << std::dec << "  >>  payload size "
                             << payload_size);

    std::unique_ptr<data_t> data(new data_t);
    data->header.payload_size = payload_size;
    data->header.seq_number = sequence_number;
    data->header.source_id = m_board_id;
    data->header.timestamp = static_cast<uint64_t>(timestamp.count());
    memset(data->payload, 0xFE, payload_size);

    // ready to be sent to EB
    auto binary = daqling::utilities::Binary(static_cast<const void *>(data.get()), total_size);

    // print binary
    // INFO("\n" << binary);

    while (!m_connections.put(0, binary) && m_run) {
      WARNING("put() failed. Trying again");
      std::this_thread::sleep_for(1ms);
    };

    sequence_number++;
    if (sequence_number == UINT32_MAX) {
      ERROR("Reached maximum sequence number! That's enough for an example...");
      throw;
    }
    std::this_thread::sleep_for(m_delay_us);
  }
  DEBUG("Runner stopped");
}
