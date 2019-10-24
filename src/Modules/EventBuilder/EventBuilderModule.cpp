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
/// \endcond

#include "Common/DataFormat.hpp"
#include "EventBuilderModule.hpp"

using namespace std::chrono_literals;

EventBuilderModule::EventBuilderModule() : eventmap_size{0} {
  DEBUG("With config: " << m_config.dump() << " getState: " << this->getState());
  m_nreceivers = m_config.getConnections()["receivers"].size();
}

EventBuilderModule::~EventBuilderModule() {}

void EventBuilderModule::configure() {
  DAQProcess::configure();

  if (m_statistics) {
    // Register statistical variables
    m_statistics->registerMetric<std::atomic<size_t>>(&eventmap_size, "EventMap-Size",
                                                      daqling::core::metrics::LAST_VALUE);
  }
}

void EventBuilderModule::start(unsigned run_num) {
  DAQProcess::start(run_num);
  DEBUG("getState: " << getState());
}

void EventBuilderModule::stop() {
  DAQProcess::stop();
  DEBUG("getState: " << this->getState());
}

void EventBuilderModule::runner() {
  DEBUG("Running...");

  std::unordered_map<uint32_t, std::vector<daqling::utilities::Binary>> events;

  while (m_run) {
    bool received = false;
    for (unsigned ch = 0; ch < m_nreceivers; ch++) {
      daqling::utilities::Binary b;
      if (m_connections.get(ch, std::ref(b))) {
        unsigned seq_number;
        data_t *d = static_cast<data_t *>(b.data());
        seq_number = d->header.seq_number;
        events[seq_number].push_back(b);
        received = true;
        if (m_statistics) {
          eventmap_size = events.size();
        }
        if (events[seq_number].size() == m_nreceivers) {
          DEBUG("complete event");
          daqling::utilities::Binary out;
          for (auto &c : events[seq_number]) {
            out += c;
          }
          m_connections.put(m_nreceivers, out);
          events.erase(seq_number);
        }
      }
    }
    if (!received) {
      std::this_thread::sleep_for(10ms);
    }
  }
  DEBUG("Runner stopped");
}
