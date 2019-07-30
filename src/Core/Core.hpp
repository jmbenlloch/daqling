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

#ifndef DAQLING_CORE_CORE_HPP
#define DAQLING_CORE_CORE_HPP

/// \cond
#include <atomic>
/// \endcond

#include "PluginManager.hpp"

namespace daqling {
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
  daqling::core::ConnectionManager &m_connections = daqling::core::ConnectionManager::instance();
  // Command exchange
  daqling::core::Command &m_command = daqling::core::Command::instance();
  // JSON Configuration map
  daqling::core::Configuration &m_config = daqling::core::Configuration::instance();
  // Plugin manager
  daqling::core::PluginManager &m_plugin = daqling::core::PluginManager::instance();
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_CORE_HPP
