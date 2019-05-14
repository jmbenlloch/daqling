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

#ifndef DAQ_CORE_CORE_HPP_
#define DAQ_CORE_CORE_HPP_

/// \cond
#include <atomic>
/// \endcond

#include "Core/PluginManager.hpp"

namespace daq {
namespace core {

class Core {
 public:
  Core(int port, std::string protocol, std::string address)
      : m_port{port}, m_protocol{protocol}, m_address{address} {};
  ~Core(){};

  bool setupCommandPath();
  bool startCommandHandler();
  bool getShouldStop();
  std::mutex *getMutex() { return m_command.getMutex(); };
  std::condition_variable *getCondVar() { return m_command.getCondVar(); };

 private:
  int m_port;
  std::string m_protocol;
  std::string m_address;

  std::thread m_cmdHandler;

  // ZMQ ConnectionManager
  daq::core::ConnectionManager &m_connections = daq::core::ConnectionManager::instance();
  // Command exchange
  daq::core::Command &m_command = daq::core::Command::instance();
  // JSON Configuration map
  daq::core::Configuration &m_config = daq::core::Configuration::instance();
  // Plugin manager
  daq::core::PluginManager &m_plugin = daq::core::PluginManager::instance();
};

}  // namespace core
}  // namespace daq

#endif
