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

namespace daqling::core {

using namespace std::placeholders;

class DAQProcess {
public:
  DAQProcess(){};

  virtual ~DAQProcess(){};

  /* use virtual otherwise linker will try to perform static linkage */
  virtual void configure() {
    setupStatistics();
    if (m_stats_on) {
      m_statistics->start();
    }
    m_state = "ready";
  };

  virtual void start(unsigned run_num) {
    m_run_number = run_num;
    DEBUG("run number " << m_run_number);
    m_run = true;
    m_state = "running";
    m_runner_thread = std::thread(&DAQProcess::runner, this);
  };

  virtual void stop() {
    m_run = false;
    if (running()) {
      m_runner_thread.join();
    }
    m_state = "ready";
  };

  virtual void runner() = 0;

  /**
   * Runs a registered custom command named `key`, if available.
   * Returns whether a command was found and run.
   */
  bool command(const std::string &key, const std::string &arg) noexcept {
    if (auto cmd = m_commands.find(key); cmd != m_commands.end()) {
      DEBUG("Command '" << key << "' registered. Running...");
      cmd->second.first(arg);
      DEBUG("Moving to state " << cmd->second.second);
      m_state = cmd->second.second;
      return true;
    }

    DEBUG("No command '" << key << "' registered");
    return false;
  }

  std::string getState() { return m_state; }

  bool setupStatistics() { // TODO

    auto statsURI = m_config.getSettings()["stats_uri"];
    auto influxDbURI = m_config.getSettings()["influxDb_uri"];
    auto influxDbName = m_config.getSettings()["influxDb_name"];
    std::string name = m_config.getConfig()["name"];
    auto numConnections = m_config.getNumConnections();

    INFO("Setting up statistics on: " << statsURI);
    if ((statsURI == "" || statsURI == nullptr) && (influxDbURI == "" || influxDbURI == nullptr)) {
      INFO("No Statistics settings were provided... Running without stats.");
      m_stats_on = false;
      return false;
    } else {
      if (statsURI != "" && statsURI != nullptr) {
        if (!m_connections.setupStatsConnection(1, statsURI)) {
          ERROR("Connection setup failed for Statistics publishing!");
          return false;
        }
      }
      m_statistics = std::make_unique<Statistics>(m_connections.getStatSocket());
      for (unsigned ch = 0; ch < numConnections; ch++) {
        m_statistics->registerMetric<std::atomic<size_t>>(
            &m_connections.getQueueStat(ch), name + "-ch" + std::to_string(ch) + "-QueueSizeGuess",
            daqling::core::metrics::LAST_VALUE);
        m_statistics->registerMetric<std::atomic<size_t>>(
            &m_connections.getMsgStat(ch), name + "-ch" + std::to_string(ch) + "-NumMessages",
            daqling::core::metrics::RATE);
      }
      if (statsURI != "" && statsURI != nullptr) {
        m_statistics->setZMQpublishing(true);
        m_stats_on = true;
      }
      if (influxDbURI != "" && influxDbURI != nullptr) {
        m_statistics->setInfluxDBname(influxDbName);
        m_statistics->setInfluxDBuri(influxDbURI);
        m_statistics->setInfluxDBsending(true);
        m_stats_on = true;
      }
    }
    return true;
  }

  bool running() const { return m_runner_thread.joinable(); }

protected:
  /**
   * Registers a custom command under the name `cmd`.
   * Returns whether the command was inserted (false meaning that command `cmd` already exists)
   */
  template <typename Function, typename... Args>
  bool registerCommand(const std::string &cmd, const std::string &target_state, Function &&f,
                       Args &&... args) {
    if (m_state == "running") {
      throw std::logic_error("commands cannot be registered during runtime.");
    }

    return m_commands.emplace(cmd, std::make_pair(std::bind(f, args...), target_state)).second;
  }

  // ZMQ ConnectionManager
  daqling::core::ConnectionManager &m_connections = daqling::core::ConnectionManager::instance();
  // JSON Configuration map
  daqling::core::Configuration &m_config = daqling::core::Configuration::instance();

  // Stats
  bool m_stats_on;
  std::unique_ptr<Statistics> m_statistics;

  std::string m_state;
  std::atomic<bool> m_run;
  std::thread m_runner_thread;
  unsigned m_run_number;

private:
  std::map<const std::string,
           std::pair<std::function<void(const std::string &)>, const std::string>>
      m_commands;
};

} // namespace daqling::core

#endif // DAQLING_CORE_DAQPROCESS_HPP
