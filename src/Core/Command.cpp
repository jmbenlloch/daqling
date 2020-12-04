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

#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

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
    try {
      auto &command = daqling::core::Command::instance();
      response = command.getState();
    } catch (std::exception const &e) {
      ERROR("Exception: " << e.what());
      response = "Failure " + std::string(e.what());
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

    auto &command = daqling::core::Command::instance();
    auto &cm = daqling::core::ConnectionManager::instance();
    auto &plugin = daqling::core::PluginManager::instance();
    std::string entry_state = command.getState();
    try {
      if (command.getState() != "booted") {
        throw invalid_command();
      }
      command.setState("configuring");
      auto &cfg = Configuration::instance();
      cfg.load(argument);
      DEBUG("Get config: " << argument);

      auto type = cfg.get<std::string>("type");
      DEBUG("Loading type: " << type);
      if (!plugin.load(type)) {
        ERROR("Plugin load failure!");
        throw module_loading_failure();
      }

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
          if (!cm.addReceiverChannel(it["chid"], dir, connStr.str(), 1000, it["filter"],
                                     it["filter_size"])) {
            ERROR("addChannel failure!");
            throw connection_failure();
          }

        } else {
          if (!cm.addReceiverChannel(it["chid"], dir, connStr.str(), 1000)) {
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
        if (!cm.addSenderChannel(it["chid"], dir, connStr.str(), 1000)) {
          ERROR("addChannel failure!");
          throw connection_failure();
        }
      }

      plugin.configure();
      command.setState("ready");
      response = "Success";
    } catch (std::exception const &e) {
      response = "Failure: " + std::string(e.what());
      ERROR("Exception: " << e.what());
      command.setState(entry_state);
    }
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
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (command.getState() != "ready") {
        throw invalid_command();
      }
      command.setState("unconfiguring");
      while (cm.getNumOfReceiverChannels() > 0) {
        cm.removeReceiverChannel(cm.getNumOfReceiverChannels() - 1);
      }
      while (cm.getNumOfSenderChannels() > 0) {
        cm.removeSenderChannel(cm.getNumOfSenderChannels() - 1);
      }

      cm.unsetStatsConnection();
      plugin.unload();
      command.setState("booted");
      response = "Success";
    } catch (std::exception const &e) {
      response = "Failure " + std::string(e.what());
      ERROR("Exception: " << e.what());
      command.setState(entry_state);
    }
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
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (std::string s = command.getState(); s != "ready" or s == "running")
        throw invalid_command();
      command.setState("starting");
      cm.start();
      plugin.start(run_num);
      command.setState("running");
      response = "Success";
    } catch (std::exception const &e) {
      response = "Failure " + std::string(e.what());
      ERROR("Exception: " << e.what());
      command.setState(entry_state);
    }
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
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (command.getState() != "running") {
        throw invalid_command();
      }
      command.setState("stopping");
      plugin.stop();
      cm.stop();
      command.setState("ready");
      response = "Success";
    } catch (std::exception const &e) {
      response = "Failure";
      ERROR("Exception: " << e.what());
      command.setState("error");
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class down : public xmlrpc_c::method {
public:
  down() {}

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) {
    std::string response;
    paramList.verifyEnd(0);
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (std::string s = command.getState(); s != "booted" and s != "ready" and s != "running") {
        throw invalid_command();
      }
      command.setState("shutting");
      command.stop_and_notify();
      command.setState("added");
      response = "Success";
    } catch (std::exception const &e) {
      response = "Failure: " + std::string(e.what());
      ERROR("Exception: " << e.what());
      command.setState(entry_state);
    }
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
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (plugin.isCommandRegistered(command_name)) {
        command.setState(plugin.getCommandTransitionState(command_name));
        plugin.command(command_name, argument);
        command.setState(plugin.getCommandTargetState(command_name));
        response = "Success";
      } else {
        ERROR("Command " << command_name << " is not registered");
        response = "Failure: " + command_name + " is not registered";
      }
    } catch (std::exception const &e) {
      response = "Failure: " + std::string(e.what());
      ERROR("Exception: " << e.what());
      command.setState(entry_state);
    }
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
    ERROR("Exception: " << e.what());
  }
}