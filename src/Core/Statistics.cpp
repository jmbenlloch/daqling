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

#include "Statistics.hpp"

using namespace daqling::core;

Statistics::Statistics(nlohmann::json &j, unsigned interval) : m_interval{interval} {
  m_stop_thread = false;
  m_influxDb = false;
  m_zmq_publisher = false;
  m_is_stats_setup = false;
  m_stats_on = false;
  auto statsURI = j["stats_uri"];
  auto influxDbURI = j["influxDb_uri"];
  auto influxDbName = j["influxDb_name"];

  ERS_INFO("Setting up statistics on: " << statsURI);
  if ((statsURI.empty() || statsURI == nullptr) &&
      (influxDbURI.empty() || influxDbURI == nullptr)) {
    ERS_INFO("No Statistics settings were provided... Running without stats.");
    m_stats_on = false;
  }
  if (!statsURI.empty() && statsURI != nullptr) {
    try {
      if (setupStatsConnection(1, statsURI)) {
        this->setZMQpublishing(true);
        m_stats_on = true;
      } else {
        this->setZMQpublishing(false);
      }
    } catch (ers::Issue &i) {
      throw core::CannotSetupStatPublishing(ERS_HERE, i);
    }
  }

  if (!influxDbURI.empty() && influxDbURI != nullptr) {
    this->setInfluxDBname(influxDbName);
    this->setInfluxDBuri(influxDbURI);
    this->setInfluxDBsending(true);
    m_stats_on = true;
  }
  try {
    m_name = m_config.getName();
  } catch (const std::exception &) {
    m_name = "";
  }
}

Statistics::~Statistics() {
  ERS_DEBUG(0, "HI FROM DESTRUCTOR");
  std::unique_lock<std::mutex> lck(m_mtx);
  m_reg_metrics.clear();
  m_reg_metrics.shrink_to_fit();
  lck.unlock();
  if (m_stat_thread.joinable()) {
    m_stop_thread = true;
    m_stat_thread.join();
  }
}

bool Statistics::setupStatsConnection(uint8_t ioT, const std::string &connStr) {
  if (m_is_stats_setup) {
    ERS_INFO(" Statistics socket is already online... Won't do anything.");
    return false;
  }
  try {
    m_stats_context = std::make_unique<zmq::context_t>(ioT);
    m_stat_socket = std::make_unique<zmq::socket_t>(*(m_stats_context.get()), ZMQ_PUB);
    m_stat_socket->connect(connStr);
    ERS_INFO(" Statistics are published on: " << connStr);
  } catch (std::exception &e) {
    throw CannotAddStatsChannel(ERS_HERE, e.what());
    return false;
  }
  m_is_stats_setup = true;
  return true;
}

bool Statistics::unsetStatsConnection() {
  ERS_DEBUG(0, "HI FROM UNSET");
  if (m_stat_thread.joinable()) {
    m_stop_thread = true;
    m_stat_thread.join();
  }
  if (m_zmq_publisher) {
    m_stat_socket.reset();
    m_stats_context.reset();
    m_is_stats_setup = false;
  }
  return true;
}

bool Statistics::configure(unsigned interval) {
  m_stop_thread = false;
  m_interval = interval;
  return true;
}

void Statistics::start() {
  ERS_INFO("Start");
  m_stat_thread = std::thread(&Statistics::CheckStatistics, this);
}

void Statistics::CheckStatistics() {
  ERS_INFO("Statistics thread about to spawn...");

  while (!m_stop_thread) {
    std::unique_lock<std::mutex> lck(m_mtx);
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
    lck.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
  }

} // CheckStatistics
