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

/**
 * @file DAQProcess.hpp
 * @brief Base class for Modules loaded via the PluginManager
 * @date 2019-02-20
 */
 
#ifndef DAQLING_CORE_DAQPROCESS_HPP
#define DAQLING_CORE_DAQPROCESS_HPP

#include "Configuration.hpp"
#include "ConnectionManager.hpp"
#include "Statistics.hpp"
#include "Utils/Logging.hpp"

namespace daqling {
namespace core {

class DAQProcess {
 public:
  DAQProcess() {};

  virtual ~DAQProcess(){};

  /* use virtual otherwise linker will try to perform static linkage */
  virtual void configure() {
    setupStatistics();
    if (m_stats_on) {
      m_statistics->start();
    }
    m_state = "ready";
  };

  virtual void start() {
    m_run = true;
    m_runner_thread = std::thread(&DAQProcess::runner, this);
    m_state = "running";
  };

  virtual void stop() {
    m_run = false;
    m_runner_thread.join();
    m_state = "ready";
  };

  virtual void runner() = 0;

  std::string getState() { return m_state; }

  bool setupStatistics() { // TODO

    auto statsURI = m_config.getConfig()["settings"]["stats_uri"];
    auto influxDbURI = m_config.getConfig()["settings"]["influxDb_uri"];
    auto influxDbName = m_config.getConfig()["settings"]["influxDb_name"];
    INFO("Setting up statistics on: " << statsURI);
    if ((statsURI == "" || statsURI == nullptr) && (influxDbURI == "" || influxDbURI == nullptr)){
      INFO("No Statistics settings were provided... Running without stats.");
      m_stats_on = false;
      return false;
    } 
    else {
      if(statsURI != "" && statsURI != nullptr){
        if ( !m_connections.setupStatsConnection(1, statsURI) ) {
          ERROR("Connection setup failed for Statistics publishing!");
          return false;
        }
      }
      
      m_statistics = std::make_unique<Statistics>(m_connections.getStatSocket());
      m_statistics->registerVariable<std::atomic<size_t>, size_t >(&m_connections.getQueueStat(1), "CHN0-QueueSizeGuess", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::SIZE);
      m_statistics->registerVariable<std::atomic<size_t>, size_t >(&m_connections.getMsgStat(1), "CHN0-NumMessages", daqling::core::metrics::LAST_VALUE, daqling::core::metrics::SIZE);
      if(statsURI != "" && statsURI != nullptr){
        m_statistics->setZQMpublishing(true);
        m_stats_on = true;
      }
      if(influxDbURI != "" && influxDbURI != nullptr){
        m_statistics->setInfluxDBname(influxDbName);
        m_statistics->setInfluxDBuri(influxDbURI);
        m_statistics->setInfluxDBsending(true);
        m_stats_on = true;
      }
    }
    return true;
  } 

 protected:
  // ZMQ ConnectionManager
  daqling::core::ConnectionManager& m_connections = daqling::core::ConnectionManager::instance();
  // JSON Configuration map
  daqling::core::Configuration& m_config = daqling::core::Configuration::instance();

  // Stats
  bool m_stats_on;
  std::unique_ptr<Statistics> m_statistics;

  std::string m_state;
  std::atomic<bool> m_run;
  std::thread m_runner_thread;
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_DAQPROCESS_HPP
