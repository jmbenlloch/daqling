/**
 * Copyright (C) 2019-2021 CERN
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

#include <thread>

#include "Command.hpp"
#include "Configuration.hpp"
#include "ConnectionLoader.hpp"
#include "ConnectionManager.hpp"
#include "ModuleLoader.hpp"

using namespace daqling::core;
using namespace std::chrono_literals;

class status : public xmlrpc_c::method {
public:
  status() = default;

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) override {
    std::string response;
    paramList.verifyEnd(0);
    try {
      auto &command = daqling::core::Command::instance();
      response = command.getState();
    } catch (ers::Issue &i) {
      ers::error(CommandIssue(ERS_HERE, i));
      response = "Failure: " + std::string(i.message());
    } catch (std::exception const &e) {
      ers::fatal(UnknownException(ERS_HERE, e.what()));
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class configure : public xmlrpc_c::method {
public:
  configure() = default;

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) override {
    std::string response;
    std::string argument = paramList.getString(0);
    paramList.verifyEnd(1);

    auto &command = daqling::core::Command::instance();
    auto &cm = daqling::core::ConnectionManager::instance();
    auto &plugin = daqling::core::ModuleLoader::instance();

    std::string entry_state = command.getState();
    try {
      if (command.getState() != "booted") {
        throw InvalidCommand(ERS_HERE);
      }
      command.setState("configuring");
      auto &cfg = Configuration::instance();
      cfg.load(argument);
      ERS_DEBUG(0, "Get config: " << argument);

      auto type = cfg.get<std::string>("type");
      ERS_DEBUG(0, "Loading type: " << type);
      try {
        plugin.load(type);

      } catch (ers::Issue &i) {
        throw CannotLoadPlugin(ERS_HERE, type.c_str(), i);
      }

      auto j = cfg.getConfig();
      auto rcvs = j["connections"]["receivers"];
      ERS_DEBUG(0, "receivers empty " << rcvs.empty());
      for (auto &it : rcvs) {
        ERS_DEBUG(0, "key" << it);
        try {
          cm.addReceiverChannel(it);
        } catch (ers::Issue &i) {
          // couldn't add receiver issue
          throw AddChannelFailed(ERS_HERE, it["chid"].get<uint>(), i);
        }
      }

      auto sndrs = j["connections"]["senders"];
      ERS_DEBUG(0, "senders empty " << sndrs.empty());
      for (auto &it : sndrs) {
        ERS_DEBUG(0, "key" << it);
        try {
          cm.addSenderChannel(it);
        } catch (ers::Issue &i) {
          // couldn't add receiver issue
          throw AddChannelFailed(ERS_HERE, it["chid"].get<uint>(), i);
        }
      }

      plugin.configure();
      command.setState("ready");
      response = "Success";
    } catch (ers::Issue &i) {
      ers::error(CommandIssue(ERS_HERE, i));
      response = "Failure: " + std::string(i.message());
      command.setState(entry_state);
    } catch (std::exception const &e) {
      ers::fatal(UnknownException(ERS_HERE, e.what()));
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class unconfigure : public xmlrpc_c::method {
public:
  unconfigure() = default;

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) override {
    std::string response;
    paramList.verifyEnd(0);
    auto &plugin = daqling::core::ModuleLoader::instance();
    auto &cm = daqling::core::ConnectionManager::instance();
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (command.getState() != "ready") {
        throw InvalidCommand(ERS_HERE);
      }
      command.setState("unconfiguring");
      plugin.unconfigure();
      while (!cm.getReceiverMap().empty()) {
        for (auto & [ ch, receiver ] : cm.getReceiverMap()) {
          cm.removeReceiverChannel(ch);
        }
      }
      while (!cm.getSenderMap().empty()) {
        for (auto & [ ch, sender ] : cm.getSenderMap()) {
          cm.removeSenderChannel(ch);
        }
      }
      plugin.unload();
      command.setState("booted");
      response = "Success";
    } catch (ers::Issue &i) {
      response = "Failure: " + std::string(i.message());
      ers::error(CommandIssue(ERS_HERE, i));
      command.setState(entry_state);
    } catch (std::exception const &e) {
      ers::fatal(UnknownException(ERS_HERE, e.what()));
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class start : public xmlrpc_c::method {
public:
  start() = default;

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) override {
    std::string response;
    const auto run_num = static_cast<unsigned>(paramList.getInt(0));
    paramList.verifyEnd(1);
    auto &plugin = daqling::core::ModuleLoader::instance();
    auto &cm = daqling::core::ConnectionManager::instance();
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (std::string s = command.getState(); s != "ready" or s == "running") {
        throw InvalidCommand(ERS_HERE);
      }
      command.setState("starting");
      cm.start();
      plugin.start(run_num);
      command.setState("running");
      response = "Success";
    } catch (ers::Issue &i) {
      response = "Failure: " + std::string(i.message());
      ers::error(CommandIssue(ERS_HERE, i));
      command.setState(entry_state);
    } catch (std::exception const &e) {
      ers::fatal(UnknownException(ERS_HERE, e.what()));
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class stop : public xmlrpc_c::method {
public:
  stop() = default;

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) override {
    std::string response;
    paramList.verifyEnd(0);
    auto &plugin = daqling::core::ModuleLoader::instance();
    auto &cm = daqling::core::ConnectionManager::instance();
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (command.getState() != "running") {
        throw InvalidCommand(ERS_HERE);
      }
      command.setState("stopping");
      plugin.stop();
      cm.stop();
      command.setState("ready");
      response = "Success";
    } catch (ers::Issue &i) {
      response = "Failure";
      ers::error(CommandIssue(ERS_HERE, i));
      command.setState(entry_state);
    } catch (std::exception const &e) {
      ers::fatal(UnknownException(ERS_HERE, e.what()));
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class down : public xmlrpc_c::method {
public:
  down() = default;

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) override {
    std::string response;
    paramList.verifyEnd(0);
    auto &plugin = daqling::core::ModuleLoader::instance();
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (std::string s = command.getState(); s != "booted" and s != "ready" and s != "running") {
        throw InvalidCommand(ERS_HERE);
      }
      command.setState("shutting");
      plugin.unconfigure();
      command.stop_and_notify();
      command.setState("added");
      response = "Success";
    } catch (ers::Issue &i) {
      response = "Failure: " + std::string(i.message());
      ers::error(CommandIssue(ERS_HERE, i));
      command.setState(entry_state);
    } catch (std::exception const &e) {
      ers::fatal(UnknownException(ERS_HERE, e.what()));
    }
    *retvalP = xmlrpc_c::value_string(response);
  };
};

class custom : public xmlrpc_c::method {
public:
  custom() = default;

  void execute(xmlrpc_c::paramList const &paramList, xmlrpc_c::value *const retvalP) override {
    std::string response;
    const std::string command_name = paramList.getString(0);
    const std::string argument = paramList.getString(1);
    paramList.verifyEnd(2);
    auto &plugin = daqling::core::ModuleLoader::instance();
    auto &command = daqling::core::Command::instance();
    std::string entry_state = command.getState();
    try {
      if (plugin.isCommandRegistered(command_name)) {
        command.setState(plugin.getCommandTransitionState(command_name));
        plugin.command(command_name, argument);
        command.setState(plugin.getCommandTargetState(command_name));
        response = "Success";
      } else {
        throw UnregisteredCommand(ERS_HERE, command_name.c_str());
      }
    } catch (ers::Issue &i) {
      response = "Failure: " + std::string(i.message());
      ers::error(CommandIssue(ERS_HERE, i));
      command.setState(entry_state);
    } catch (std::exception const &e) {
      ers::fatal(UnknownException(ERS_HERE, e.what()));
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
    ERS_INFO("Server set up on port: " << port);
  } catch (ers::Issue &i) {
    ers::error(CommandIssue(ERS_HERE, i));
  } catch (std::exception const &e) {
    ers::fatal(UnknownException(ERS_HERE, e.what()));
  }
}