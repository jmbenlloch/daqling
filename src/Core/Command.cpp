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

/// \cond
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>
/// \endcond

#include "Command.hpp"
#include "Configuration.hpp"
#include "ConnectionManager.hpp"
#include "PluginManager.hpp"

using namespace daqling::core;
using namespace std::chrono_literals;

bool daqling::core::Command::executeCommand(std::string &response) {
  // INFO("Loaded configuration");
  // auto command = cfg.get<std::string>("command");
  auto command = nlohmann::json::parse(m_command)["command"];
  DEBUG("Get command: " << command);
  auto &m_plugin = daqling::core::PluginManager::instance();
  auto &cm = daqling::core::ConnectionManager::instance();

  try {
    if (command == "configure") {
      if (m_plugin.getLoaded())
        throw invalid_command();
      auto &cfg = Configuration::instance();
      cfg.load(m_argument);
      DEBUG("Get config: " << m_argument);

      auto type = cfg.get<std::string>("type");
      DEBUG("Loading type: " << type);

      auto j = cfg.getConfig();
      auto rcvs = j["connections"]["receivers"];
      DEBUG("receivers empty " << rcvs.empty());
      for (auto &it : rcvs) {
        DEBUG("key" << it);
        std::ostringstream connStr;
        ConnectionManager::EDirection dir;
        if (it["type"] == "pair") {
          dir = ConnectionManager::EDirection::CLIENT;
        } else if (it["type"] == "pubsub") {
          dir = ConnectionManager::EDirection::SUBSCRIBER;
        } else {
          ERROR("Unrecognized socket type");
          throw connection_failure();
        }
        if (it["transport"] == "ipc") {
          std::string path = it["path"];
          connStr << "ipc://" << path;
        } else if (it["transport"] == "tcp") {
          std::string host = it["host"];
          connStr << "tcp://" << host << ":" << it["port"];
        } else {
          ERROR("Unrecognized transport type");
          throw connection_failure();
        }
        if (!cm.addChannel(it["chid"], dir, connStr.str(), 10000)) {
          ERROR("addChannel failure!");
          throw connection_failure();
        }
      }

      auto sndrs = j["connections"]["senders"];
      DEBUG("senders empty " << sndrs.empty());
      for (auto &it : sndrs) {
        DEBUG("key" << it);
        std::ostringstream connStr;
        ConnectionManager::EDirection dir;
        if (it["type"] == "pair") {
          dir = ConnectionManager::EDirection::SERVER;
        } else if (it["type"] == "pubsub") {
          dir = ConnectionManager::EDirection::PUBLISHER;
        } else {
          ERROR("Unrecognized socket type");
          throw connection_failure();
        }
        if (it["transport"] == "ipc") {
          std::string path = it["path"];
          connStr << "ipc://" << path;
        } else if (it["transport"] == "tcp") {
          std::string host = it["host"];
          connStr << "tcp://" << host << ":" << it["port"];
        } else {
          ERROR("Unrecognized transport type");
          throw connection_failure();
        }
        if (!cm.addChannel(it["chid"], dir, connStr.str(), 10000)) {
          ERROR("addChannel failure!");
          throw connection_failure();
        }
      }

      if (!m_plugin.load(type)) {
        ERROR("Plugin load failure!");
        throw connection_failure();
      }
      response = "Success";
      m_plugin.configure();
    } else if (command == "unconfigure") {
      if (!m_plugin.getLoaded())
        throw invalid_command();
      while (cm.getNumOfChannels() > 0) {
        cm.removeChannel(cm.getNumOfChannels());
      }
      cm.unsetStatsConnection();
      m_plugin.unload();
      response = "Success";
    } else if (command == "start") {
      if (!m_plugin.getLoaded() || m_plugin.getState() == "running")
        throw invalid_command();
      cm.start();
      m_plugin.start(static_cast<unsigned>(std::stoi(m_argument)));
      response = "Success";
    } else if (command == "stop") {
      if (!m_plugin.getLoaded() || m_plugin.getState() != "running")
        throw invalid_command();
      m_plugin.stop();
      cm.stop();
      response = "Success";
    } else if (command == "shutdown") {
      stop_and_notify();
      response = "Success";
    } else if (command == "status") {
      if (m_plugin.getLoaded()) {
        response = m_plugin.getState();
      } else {
        response = "booted";
      }
    } else {
      if (m_plugin.command(command, m_argument)) {
        response = "Success";
      } else {
        return false;
      }
    }
  } catch (connection_failure &) {
    response = "Failure";
    stop_and_notify();
    return false;
  } catch (invalid_command &) {
    WARNING("Invalid command " << command << ". Return Failure.");
    response = "Failure";
    return false;
  }

  return true; // TODO put some meaning or return void
}

bool daqling::core::Command::handleCommand() {

  DEBUG("CommandThread  ->>> Should handle command: " << m_command);
  std::string response;
  [[maybe_unused]] bool ret = executeCommand(response);
  setResponse(response);
  setHandled(true);

  return true;
}

// template <typename TValue, typename TPred>
// BinarySearchTree<TValue, TPred>::BinarySearchTree()

/*
template <class ST>
ConnectionManager<ST>::ConnectionManager(m_token)
{

}
*/

/*
template <class ST>
ConnectionManager<ST>::~ConnectionManager() {

}
*/
