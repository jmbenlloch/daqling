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

#ifndef DAQLING_CORE_COMMAND_HPP
#define DAQLING_CORE_COMMAND_HPP

#include "Utils/Logging.hpp"
#include "Utils/ReusableThread.hpp"
#include "Utils/Singleton.hpp"
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

namespace daqling {
namespace core {

struct connection_failure : public std::exception {
  const char *what() const throw() { return "Connection failure"; }
};
struct invalid_command : public std::exception {
  const char *what() const throw() { return "Invalid command"; }
};
struct module_loading_failure : public std::exception {
  const char *what() const throw() { return "Module loading failure"; }
};

class Command : public daqling::utilities::Singleton<Command> {
public:
  Command()
      : m_should_stop{false},
        m_handled(false), m_state{"booted"}, m_command{""}, m_argument{""}, m_response{""} {}
  ~Command() {
    m_server_p->terminate();
    m_cmd_handler.join();
  }

  void setState(const std::string &state) {
    DEBUG("Setting state: " << state);
    const std::lock_guard<std::mutex> lock(m_state_mtx);
    m_state = state;
  }

  std::string getState() {
    const std::lock_guard<std::mutex> lock(m_state_mtx);
    return m_state;
  }

  void setupServer(unsigned port);

  bool getShouldStop() { return m_should_stop; }
  std::mutex *getMutex() { return &m_mtx; }
  std::condition_variable *getCondVar() { return &m_cv; }

  void stop_and_notify() {
    DEBUG("Shutting down...");
    std::lock_guard<std::mutex> lk(m_mtx);
    m_should_stop = true;
    m_cv.notify_one();
  }

private:
  bool m_should_stop;
  bool m_handled;
  std::string m_state;
  std::string m_command;
  std::string m_argument;
  std::string m_response;
  std::mutex m_mtx;
  std::mutex m_state_mtx;
  std::condition_variable m_cv;
  std::thread m_cmd_handler;
  xmlrpc_c::registry m_registry;
  std::map<std::string, xmlrpc_c::methodPtr> m_method_pointers;
  std::unique_ptr<xmlrpc_c::serverAbyss> m_server_p;
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_COMMAND_HPP
