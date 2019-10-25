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

EventBuilderModule::EventBuilderModule() : m_eventmap_size{0}, m_complete_ev_size_guess{0} {
  DEBUG("With config: " << m_config.dump() << " getState: " << this->getState());
  m_nreceivers = m_config.getConnections()["receivers"].size();
}

EventBuilderModule::~EventBuilderModule() {}

void EventBuilderModule::configure() {
  DAQProcess::configure();

  if (m_statistics) {
    // Register statistical variables
    m_statistics->registerMetric<std::atomic<size_t>>(&m_eventmap_size, "EventMap-Size",
                                                      daqling::core::metrics::LAST_VALUE);
    m_statistics->registerMetric<std::atomic<size_t>>(&m_complete_ev_size_guess,
                                                      "CompleteEventQueue-SizeGuess",
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
  daqling::utilities::ReusableThread rt(0);
  folly::ProducerConsumerQueue<unsigned> complete_seq(1000);
  std::mutex mtx;

  std::thread consumer{[&]() {
    while (m_run) {
      unsigned seq;
      while (!complete_seq.read(seq) && m_run)
        std::this_thread::sleep_for(10ms);
      daqling::utilities::Binary out;
      mtx.lock();
      for (auto &c : events[seq]) {
        out += c;
      }
      events.erase(seq);
      mtx.unlock();
      m_connections.put(m_nreceivers, out);
    }
  }};

  while (m_run) {
    bool received = false;
    for (unsigned ch = 0; ch < m_nreceivers; ch++) {
      daqling::utilities::Binary b;
      if (m_connections.get(ch, std::ref(b))) {
        unsigned seq_number;
        data_t *d = static_cast<data_t *>(b.data());
        seq_number = d->header.seq_number;
        mtx.lock();
        events[seq_number].push_back(b);
        if (events[seq_number].size() == m_nreceivers) {
          complete_seq.write(seq_number);
        }
        if (m_statistics) {
          m_eventmap_size = events.size();
          m_complete_ev_size_guess = complete_seq.sizeGuess();
        }
        mtx.unlock();
        received = true;
      }
    }
    if (!received) {
      std::this_thread::sleep_for(10ms);
    }
  }
  consumer.join();
  DEBUG("Runner stopped");
}
