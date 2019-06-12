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
 * @author Enrico.Gamberini@cern.ch
 * @brief Base class for Modules loaded via the PluginManager
 * @date 2019-02-20
 */
 
#ifndef DAQLING_CORE_DAQPROCESS_HPP
#define DAQLING_CORE_DAQPROCESS_HPP

#include "Core/Configuration.hpp"
#include "Core/ConnectionManager.hpp"
#include "Core/Statistics.hpp"
#include "Utilities/Common.hpp"
#include "Utilities/Logging.hpp"

#define __METHOD_NAME__ daqling::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daqling::utilities::className(__PRETTY_FUNCTION__)

namespace daqling {
namespace core {

class DAQProcess {
 public:
  DAQProcess() : m_state{"ready"} {};

  virtual ~DAQProcess(){};

  /* use virtual otherwise linker will try to perform static linkage */
  virtual void start() {
    m_run = true;
    m_runner_thread = std::make_unique<std::thread>(&DAQProcess::runner, this);
    if (m_stats_on) {
      m_statistics->start();
    }
    m_state = "running";
  };

  virtual void stop() {
    m_run = false;
    m_runner_thread->join();
    m_state = "ready";
  };

  virtual void runner() = 0;

  std::string getState() { return m_state; }

  bool setupStatistics() { // TODO

    auto statsURI = m_config.getConfig()["settings"]["stats_uri"];
    INFO(__METHOD_NAME__ << " Setting up statistics on: " << statsURI);
    if (statsURI == "" || statsURI == nullptr){
      INFO(__METHOD_NAME__ << " No Statistics settings were provided... Running without stats. ");
      m_stats_on = false;
      return false;
    } else {
      if ( !m_connections.setupStatsConnection(1, statsURI) ) {
        ERROR(__METHOD_NAME__ << " Connection setup failed for Statistics publishing! ");
        return false;
      }
      m_statistics = std::make_unique<Statistics>(m_connections.getStatSocket());
      //m_statistics->registerCoreMetric("CHN0-QueueSizeGuess", &m_connections.getQueueStat(1) );
      m_statistics->registerVariable<std::atomic<size_t>, size_t >(&m_connections.getQueueStat(1), "CHN0-QueueSizeGuess", daqling::core::LAST_VALUE, daqling::core::SIZE);
      //m_statistics->registerCoreMetric("CHN0-NumMessages", &m_connections.getMsgStat(1) );
      m_statistics->registerVariable<std::atomic<size_t>, size_t >(&m_connections.getMsgStat(1), "CHN0-NumMessages", daqling::core::LAST_VALUE, daqling::core::SIZE);
      m_stats_on = true;
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
  std::unique_ptr<std::thread> m_runner_thread;
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_DAQPROCESS_HPP
