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

#include "Statistics.hpp"

using namespace daqling::core;

Statistics::Statistics(std::unique_ptr<zmq::socket_t> &statSock, unsigned interval)
    : m_stat_socket{statSock}, m_interval{interval} {
  m_stop_thread = false;
  m_influxDb = false;
  m_zmq_publisher = false;
}

Statistics::~Statistics() {
  m_reg_metrics.clear();
  m_reg_metrics.shrink_to_fit();
  m_stop_thread = true;
  if (m_stat_thread.joinable())
    m_stat_thread.join();
}

bool Statistics::configure(unsigned interval) {
  m_interval = interval;
  return true;
}

void Statistics::start() {
  INFO("Start");
  m_stat_thread = std::thread(&Statistics::CheckStatistics, this);
}

void Statistics::CheckStatistics() {
  INFO("Statistics thread about to spawn...");

  while (!m_stop_thread) {

    for (auto &m : m_reg_metrics) {
      Metric_base *x = m.get();
      if (x->m_mtype == metrics::AVERAGE) {
        switch (x->m_vtype) {
        case metrics::FLOAT:
          accumulateValue<std::atomic<float>, float>(x);
          break;
        case metrics::INT:
          accumulateValue<std::atomic<int>, int>(x);
          break;
        case metrics::DOUBLE:
          accumulateValue<std::atomic<double>, double>(x);
          break;
        case metrics::BOOL:
          accumulateValue<std::atomic<bool>, bool>(x);
          break;
        case metrics::SIZE:
          accumulateValue<std::atomic<size_t>, size_t>(x);
          break;
        };
      }

      if (x->m_mtype == metrics::ACCUMULATE) {
        switch (x->m_vtype) {
        case metrics::FLOAT:
          accumulateValueAndReset<std::atomic<float>, float>(x);
          break;
        case metrics::INT:
          accumulateValueAndReset<std::atomic<int>, int>(x);
          break;
        case metrics::DOUBLE:
          accumulateValueAndReset<std::atomic<double>, double>(x);
          break;
        case metrics::BOOL:
          accumulateValueAndReset<std::atomic<bool>, bool>(x);
          break;
        case metrics::SIZE:
          accumulateValueAndReset<std::atomic<size_t>, size_t>(x);
          break;
        };
      }

      if (std::difftime(std::time(nullptr), x->m_timestamp) >= x->m_delta_t) {
        switch (x->m_vtype) {
        case metrics::FLOAT:
          publishValue<std::atomic<float>, float>(x);
          break;
        case metrics::INT:
          publishValue<std::atomic<int>, int>(x);
          break;
        case metrics::DOUBLE:
          publishValue<std::atomic<double>, double>(x);
          break;
        case metrics::BOOL:
          publishValue<std::atomic<bool>, bool>(x);
          break;
        case metrics::SIZE:
          publishValue<std::atomic<size_t>, size_t>(x);
          break;
        };
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
  }

} // CheckStatistics
