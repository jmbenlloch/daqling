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

bool daqling::core::Command::startCommandHandler() {
  // m_commandHandler = std::make_unique<daqling::utilities::ReusableThread>(10);
  unsigned tid = 1;
  bool rv = false;
  m_commandFunctors.push_back([&, tid] {
    DEBUG("CommandThread  ->>> Should handle command: " << m_command);
    std::string response;
    rv = executeCommand(response);
    setResponse(response);
    setHandled(true);
  });
  return rv;
}

bool daqling::core::Command::executeCommand(std::string& response) {
  // INFO("Loaded configuration");
  // auto command = cfg.get<std::string>("command");
  auto command = nlohmann::json::parse(m_command)["command"];
  DEBUG("Get command: " << command);
  auto& m_plugin = daqling::core::PluginManager::instance();
  auto& cm = daqling::core::ConnectionManager::instance();

  if (command == "configure") {
    auto& cfg = Configuration::instance();
    cfg.load(m_config);
    DEBUG("Get config: " << m_config);

    auto type = cfg.get<std::string>("type");
    INFO("Loading type: " << type);

    auto j = cfg.getConfig();
    auto rcvs = j["connections"]["receivers"];
    INFO("receivers empty " << rcvs.empty());
    for (auto& it : rcvs) {
      INFO("key" << it);
      if (it["type"] == "pair") {
        cm.addChannel(it["chid"], ConnectionManager::EDirection::CLIENT, 0, it["host"], it["port"],
                      10000, false);
      } else if (it["type"] == "pubsub") {
        cm.addChannel(it["chid"], ConnectionManager::EDirection::SUBSCRIBER, 0, it["host"],
                      it["port"], 10000, false);
      } else {
        ERROR("Connection type not recognized!");
        response = "Failure";
        ERROR("Shutting down...");
        stop_and_notify();
      }
    }

    auto sndrs = j["connections"]["senders"];
    INFO("senders empty " << sndrs.empty());
    for (auto& it : sndrs) {
      INFO("key" << it);
      if (it["type"] == "pair") {
        cm.addChannel(it["chid"], ConnectionManager::EDirection::SERVER, 0, it["host"], it["port"],
                      10000, false);
      } else if (it["type"] == "pubsub") {
        cm.addChannel(it["chid"], ConnectionManager::EDirection::PUBLISHER, 0, it["host"],
                      it["port"], 10000, false);
      } else {
        ERROR("Connection type not recognized!");
        response = "Failure";
        ERROR("Shutting down...");
        stop_and_notify();
      }
    }

    bool rv = m_plugin.load(type);
    if (rv == true) {
      response = "Success";
    } else {
      response = "Failure";
      ERROR("Shutting down...");
      stop_and_notify();
    }
    m_plugin.configure();

  } else if (command == "start") {
    cm.start();

    m_plugin.start();
    response = "Success";

    INFO("Started connection manager");
  } else if (command == "stop") {
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
  }
  return true;  // TODO put some meaning or return void
}

bool daqling::core::Command::handleCommand() {
  m_commandHandler->set_work(m_commandFunctors[0]);
  while (busy()) {
    std::this_thread::sleep_for(1ms);
  }
  return true;
}

bool daqling::core::Command::busy() {
  bool busy = (m_commandHandler->get_readiness() == false) ? true : false;
  return busy;
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
