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

class status : public xmlrpc_c::method {
public:
  status() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response;
    paramList.verifyEnd(0);
    auto &plugin = daqling::core::PluginManager::instance();
    if (plugin.getLoaded()) {
      response = plugin.getState();
    } else {
      response = "booted";
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class configure : public xmlrpc_c::method {
public:
  configure() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response;
    std::string argument = paramList.getString(0);
    paramList.verifyEnd(1);

    auto &cm = daqling::core::ConnectionManager::instance();
    auto &plugin = daqling::core::PluginManager::instance();
    if (plugin.getLoaded())
      throw invalid_command();
    auto &cfg = Configuration::instance();
    cfg.load(argument);
    DEBUG("Get config: " << argument);

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
      if (it.contains("filter") && it.contains("filter_size")) {
        if (!cm.addChannel(it["chid"], dir, connStr.str(), 1000, it["filter"], it["filter_size"])) {
          ERROR("addChannel failure!");
          throw connection_failure();
        }

      } else {
        if (!cm.addChannel(it["chid"], dir, connStr.str(), 1000)) {
          ERROR("addChannel failure!");
          throw connection_failure();
        }
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
      if (!cm.addChannel(it["chid"], dir, connStr.str(), 1000)) {
        ERROR("addChannel failure!");
        throw connection_failure();
      }
    }

    if (!plugin.load(type)) {
      ERROR("Plugin load failure!");
      throw connection_failure();
    }
    response = "Success";
    plugin.configure();
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class unconfigure : public xmlrpc_c::method {
public:
  unconfigure() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response;
    paramList.verifyEnd(0);
    auto &plugin = daqling::core::PluginManager::instance();
    auto &cm = daqling::core::ConnectionManager::instance();
    if (!plugin.getLoaded())
      throw invalid_command();
    while (cm.getNumOfChannels() > 0) {
      cm.removeChannel(cm.getNumOfChannels() - 1);
    }
    cm.unsetStatsConnection();
    plugin.unload();
    response = "Success";

    *retvalP = xmlrpc_c::value_string(response);
  };
};

class start : public xmlrpc_c::method {
public:
  start() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response;
    const unsigned run_num = static_cast<unsigned>(paramList.getInt(0));
    paramList.verifyEnd(1);
    auto &plugin = daqling::core::PluginManager::instance();
    auto &cm = daqling::core::ConnectionManager::instance();
    if (!plugin.getLoaded() || plugin.getState() == "running")
      throw invalid_command();
    cm.start();
    plugin.start(run_num);
    response = "Success";

    *retvalP = xmlrpc_c::value_string(response);
  };
};

class stop : public xmlrpc_c::method {
public:
  stop() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response;
    paramList.verifyEnd(0);
    auto &plugin = daqling::core::PluginManager::instance();
    auto &cm = daqling::core::ConnectionManager::instance();

    if (!plugin.getLoaded() || plugin.getState() == "ready")
      throw invalid_command();
    plugin.stop();
    cm.stop();
    response = "Success";

    *retvalP = xmlrpc_c::value_string(response);
  };
};

class down : public xmlrpc_c::method {
public:
  down() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response{"Failure"};
    paramList.verifyEnd(0);
    auto &command = daqling::core::Command::instance();
    command.stop_and_notify();
    response = "Success";
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class custom : public xmlrpc_c::method {
public:
  custom() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response;
    const std::string command_name = paramList.getString(0);
    const std::string argument = paramList.getString(1);
    paramList.verifyEnd(2);
    auto &plugin = daqling::core::PluginManager::instance();
    plugin.command(command_name, argument);
    response = "Success";
    *retvalP = xmlrpc_c::value_string(response);
  };
};

void daqling::core::Command::setupServer(unsigned port) {
  try {
    m_method_pointers.insert(std::make_pair("status", new status));
    m_method_pointers.insert(std::make_pair("down", new down));
    m_method_pointers.insert(std::make_pair("configure", new configure));
    m_method_pointers.insert(std::make_pair("unconfigure", new unconfigure));
    m_method_pointers.insert(std::make_pair("start", new start));
    m_method_pointers.insert(std::make_pair("stop", new stop));
    m_method_pointers.insert(std::make_pair("custom", new custom));
    for (auto const & [ name, pointer ] : m_method_pointers) {
      m_registry.addMethod(name, pointer);
    }

    m_server_p = std::make_unique<xmlrpc_c::serverAbyss>(
        xmlrpc_c::serverAbyss::constrOpt().registryP(&m_registry).portNumber(port));
    m_cmd_handler = std::thread([&]() { m_server_p->run(); });
    INFO("Server set up on port: " << port);
  } catch (std::exception const &e) {
    ERROR("Something bad happened!" << e.what());
  }
}