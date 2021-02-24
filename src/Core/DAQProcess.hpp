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

#include "ConnectionManager.hpp"
#include "Statistics.hpp"
#include "Utils/Ers.hpp"

namespace daqling {
#include <ers/Issue.h>

ERS_DECLARE_ISSUE(core, DAQProcessIssue, "", ERS_EMPTY)

ERS_DECLARE_ISSUE_BASE(core, CannotSetupStatPublishing, core::DAQProcessIssue,
                       "Connection setup failed for Statistics publishing!", ERS_EMPTY, ERS_EMPTY)
namespace core {

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
  };

  virtual void start(unsigned run_num) {
    m_run_number = run_num;
    ERS_DEBUG(0, "run number " << m_run_number);
    m_run = true;
    m_runner_thread = std::thread(&DAQProcess::runner, this);
  };

  virtual void stop() {
    m_run = false;
    if (running()) {
      m_runner_thread.join();
    }
  };

  virtual void runner() noexcept = 0;

  /**
   * Returns whether a command was found in register.
   */
  bool isCommandRegistered(const std::string &key) noexcept {
    if (auto cmd = m_commands.find(key); cmd != m_commands.end()) {
      return true;
    }
    ERS_WARNING("No command '" << key << "' registered");
    return false;
  }

  /**
   * Runs a registered custom command named `key`.
   */
  void command(const std::string &key, const std::string &arg) {
    auto cmd = m_commands.find(key);
    std::get<0>(cmd->second)(arg);
  }

  std::string getCommandTransitionState(const std::string &key) {
    auto cmd = m_commands.find(key);
    return std::get<1>(cmd->second);
  }

  std::string getCommandTargetState(const std::string &key) {
    auto cmd = m_commands.find(key);
    return std::get<2>(cmd->second);
  }

  bool setupStatistics() { // TODO

    auto statsURI = m_config.getMetricsSettings()["stats_uri"];
    auto influxDbURI = m_config.getMetricsSettings()["influxDb_uri"];
    auto influxDbName = m_config.getMetricsSettings()["influxDb_name"];

    ERS_INFO("Setting up statistics on: " << statsURI);
    if ((statsURI == "" || statsURI == nullptr) && (influxDbURI == "" || influxDbURI == nullptr)) {
      ERS_INFO("No Statistics settings were provided... Running without stats.");
      m_stats_on = false;
      return false;
    } else {
      if (statsURI != "" && statsURI != nullptr) {
        try {
          if (!m_connections.setupStatsConnection(1, statsURI)) {
            return false;
          }
        } catch (ers::Issue &i) {
          throw core::CannotSetupStatPublishing(ERS_HERE, i);
        }
      }
      m_statistics = std::make_unique<Statistics>(m_connections.getStatSocket());
      for (unsigned ch = 0; ch < m_config.getNumReceiverConnections(); ch++) {
        m_statistics->registerMetric<std::atomic<size_t>>(&m_connections.getReceiverQueueStat(ch),
                                                          "ReceiverCh" + std::to_string(ch) +
                                                              "-QueueSizeGuess",
                                                          daqling::core::metrics::LAST_VALUE);
        m_statistics->registerMetric<std::atomic<size_t>>(
            &m_connections.getReceiverMsgStat(ch),
            "ReceiverCh" + std::to_string(ch) + "-NumMessages", daqling::core::metrics::RATE);
      }
      for (unsigned ch = 0; ch < m_config.getNumSenderConnections(); ch++) {
        m_statistics->registerMetric<std::atomic<size_t>>(&m_connections.getSenderQueueStat(ch),
                                                          "SenderCh" + std::to_string(ch) +
                                                              "-QueueSizeGuess",
                                                          daqling::core::metrics::LAST_VALUE);
        m_statistics->registerMetric<std::atomic<size_t>>(
            &m_connections.getSenderMsgStat(ch), "SenderCh" + std::to_string(ch) + "-NumMessages",
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
  bool registerCommand(const std::string &cmd, const std::string &transition_state,
                       const std::string &target_state, Function &&f, Args &&... args) {
    // if (m_state == "running") {
    //   throw std::logic_error("commands cannot be registered during runtime.");
    // }

    return m_commands
        .emplace(cmd, std::make_tuple(std::bind(f, args...), transition_state, target_state))
        .second;
  }

  // ZMQ ConnectionManager
  daqling::core::ConnectionManager &m_connections = daqling::core::ConnectionManager::instance();
  // JSON Configuration map
  daqling::core::Configuration &m_config = daqling::core::Configuration::instance();

  // Stats
  bool m_stats_on;
  std::unique_ptr<Statistics> m_statistics;

  std::atomic<bool> m_run;
  std::thread m_runner_thread;
  unsigned m_run_number;

private:
  std::map<const std::string, std::tuple<std::function<void(const std::string &)>,
                                         const std::string, const std::string>>
      m_commands;
};
} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_DAQPROCESS_HPP
