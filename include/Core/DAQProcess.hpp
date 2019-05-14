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
 
#ifndef DAQPROCESS_HPP_
#define DAQPROCESS_HPP_

#include "Core/Configuration.hpp"
#include "Core/ConnectionManager.hpp"
#include "Utilities/Common.hpp"
#include "Utilities/Logging.hpp"

#define __METHOD_NAME__ daq::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daq::utilities::className(__PRETTY_FUNCTION__)

class DAQProcess {
 public:
  DAQProcess() : m_state{"ready"} {};

  virtual ~DAQProcess(){};

  /* use virtual otherwise linker will try to perform static linkage */
  virtual void start() {
    m_run = true;
    m_runner_thread = std::make_unique<std::thread>(&DAQProcess::runner, this);
    m_state = "running";
  };

  virtual void stop() {
    m_run = false;
    m_runner_thread->join();
    m_state = "ready";
  };

  virtual void runner() = 0;

  std::string getState() { return m_state; }

 protected:
  // ZMQ ConnectionManager
  daq::core::ConnectionManager& m_connections = daq::core::ConnectionManager::instance();
  // JSON Configuration map
  daq::core::Configuration& m_config = daq::core::Configuration::instance();

  std::string m_state;
  std::atomic<bool> m_run;
  std::unique_ptr<std::thread> m_runner_thread;
};

#endif /* DAQPROCESS_HPP_ */
