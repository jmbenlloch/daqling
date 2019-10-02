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

#ifndef STATISTICS_HPP
#define STATISTICS_HPP

/// \cond
#include <atomic>
#include <cpr/cpr.h>
#include <ctime>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
/// \endcond

#include "Metric.hpp"
#include "Utils/Logging.hpp"
#include "Utils/zhelpers.hpp"

namespace daqling {
namespace core {

class Statistics {
public:
  Statistics(std::unique_ptr<zmq::socket_t> &statSock, unsigned interval = 500);

  ~Statistics();

  bool configure(unsigned interval);

  void setZQMpublishing(bool zmq_publisher) { m_zmq_publisher = zmq_publisher; }
  void setInfluxDBsending(bool influxDb) { m_influxDb = influxDb; }
  void setInfluxDBname(std::string influxDb_name) { m_influxDb_name = influxDb_name; }
  void setInfluxDBuri(std::string influxDb_uri) { m_influxDb_uri = influxDb_uri; }

  void start();

  void registerCoreMetric(std::string name, std::atomic<size_t> *metric);

  // template <class T>
  // void registerVariable(T* pointer, std::string name, metric_type mtype, float delta_t=1) {

  //}

  template <class T, class U>
  void registerVariable(T *pointer, std::string name, metrics::metric_type mtype,
                        metrics::variable_type vtype, float delta_t = 1) {
    if (delta_t < m_interval / 1000) {
      delta_t = m_interval;
      INFO("delta_t parameter of registerVariable(...) function can not be smaller than "
           "m_interval! Setting delta_t to m_interval value.");
    }
    std::shared_ptr<Metric<T, U>> metric(new Metric<T, U>(pointer, name, mtype, vtype, delta_t));
    std::shared_ptr<Metric_base> metric_base = std::dynamic_pointer_cast<Metric_base>(metric);
    m_reg_metrics.push_back(metric_base);
  }

  template <class T, class U> void accumulateValue(Metric_base *m) {
    Metric<T, U> *metric = static_cast<Metric<T, U> *>(m);
    U value = *(metric->m_metrics_ptr);
    metric->m_values.push_back(value);
  }

  template <class T, class U> void accumulateValueAndReset(Metric_base *m) {
    Metric<T, U> *metric = static_cast<Metric<T, U> *>(m);
    U value = *(metric->m_metrics_ptr);
    *(metric->m_metrics_ptr) = 0.;
    metric->m_values.push_back(value);
  }

  template <class T, class U> void publishValue(Metric_base *m) {
    INFO("publish value");
    U value = 0;
    Metric<T, U> *metric = static_cast<Metric<T, U> *>(m);
    if (metric->m_mtype == metrics::AVERAGE) {
      U average = std::accumulate(metric->m_values.begin(), metric->m_values.end(), 0.0) /
                  metric->m_values.size();
      metric->m_values.clear();
      metric->m_values.shrink_to_fit();
      value = average;
    } else if (metric->m_mtype == metrics::ACCUMULATE) {
      U sum = std::accumulate(metric->m_values.begin(), metric->m_values.end(), 0.0);
      metric->m_values.clear();
      metric->m_values.shrink_to_fit();
      value = sum;
    } else if (metric->m_mtype == metrics::LAST_VALUE) {
      value = *(metric->m_metrics_ptr);
      metric->m_values.clear();
      metric->m_values.shrink_to_fit();
      metric->m_values.push_back(value);
    } else if (metric->m_mtype == metrics::RATE) {
      value = *(metric->m_metrics_ptr);
      U last_value = 0;
      if (metric->m_values.size() == 1) {
        last_value = metric->m_values[0];
      }
      metric->m_values.clear();
      metric->m_values.shrink_to_fit();
      metric->m_values.push_back(value);
      if (std::difftime(std::time(nullptr), metric->m_timestamp) != 0)
        value = (value - last_value) /
                static_cast<U>(std::difftime(std::time(nullptr), metric->m_timestamp));
      else {
        WARNING(
            "Too short time interval to calculate RATE! Extend delta_t parameter of your metric");
        return;
      }
    }

    metric->m_timestamp = std::time(nullptr);

    if (m_zmq_publisher) {
      std::ostringstream msg;
      msg << metric->m_name << ": " << value;
      zmq::message_t message(msg.str().size());
      memcpy(message.data(), msg.str().data(), msg.str().size());
      INFO(" MSG " << msg.str());
      bool rc = m_stat_socket->send(message);
      if (!rc)
        WARNING("Failed to publish metric: " << metric->m_name);
    }
    if (m_influxDb) {
      INFO("Sending the metric: " << metric->m_name << " value: " << std::to_string(value)
                                  << " to influxDB");
      INFO(m_influxDb_uri + m_influxDb_name);
      auto r = cpr::Post(cpr::Url{m_influxDb_uri + m_influxDb_name},
                         cpr::Payload{{metric->m_name + " value", std::to_string(value)}});

      INFO("InfluxDB response: " << r.status_code << "\t" << r.text);
    }
  }

private:
  // Thread control
  std::thread m_stat_thread;
  std::atomic<bool> m_stop_thread;

  // Config for data publishing
  std::atomic<bool> m_influxDb;
  std::atomic<bool> m_zmq_publisher;

  // Config for influxDB
  std::string m_influxDb_name;
  std::string m_influxDb_uri;

  // Publish socket ref for stats
  std::unique_ptr<zmq::socket_t> &m_stat_socket;

  // Config
  unsigned m_interval;
  std::map<std::string, std::atomic<size_t> *> m_registered_metrics;
  // std::vector<Metric_base*> m_reg_metrics;
  std::vector<std::shared_ptr<Metric_base>> m_reg_metrics;

  // Runner
  void CheckStatistics();
};

} // namespace core
} // namespace daqling

#endif // STATISTICS_HPP
