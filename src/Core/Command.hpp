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

namespace daqling {
  namespace core {

    class Command : public daqling::utilities::Singleton<Command> {
  public:
      Command()
          : m_should_stop{false}, m_handled(false), m_command{""}, m_argument{""}, m_response{""}
      {
        m_commandHandler = std::make_unique<daqling::utilities::ReusableThread>(10);
      }
      ~Command() {}

      //    std::string getResponse() { return m_response; }
      //    bool getHandled() { return m_handled; }

      bool startCommandHandler();
      bool executeCommand(std::string &response);
      bool handleCommand();

      bool getHandled() { return m_handled; }
      void setHandled(bool handled) { m_handled = handled; }
      std::string getCommand() { return m_command; }
      void setCommand(std::string command) { m_command = command; }
      std::string getArgument() { return m_argument; }
      void setArgument(std::string argument) { m_argument = argument; }
      std::string getResponse() { return m_response; }
      void setResponse(std::string response) { m_response = response; }
      bool getShouldStop() { return m_should_stop; }
      std::mutex *getMutex() { return &m_mtx; }
      std::condition_variable *getCondVar() { return &m_cv; }

  private:
      bool busy();
      bool m_should_stop;
      bool m_handled;
      std::string m_command;
      std::string m_argument;
      std::string m_response;
      std::mutex m_mtx;
      std::condition_variable m_cv;

      // Command handler
      std::unique_ptr<daqling::utilities::ReusableThread> m_commandHandler;
      std::vector<std::function<void()>> m_commandFunctors;

      void stop_and_notify()
      {
        WARNING("Shutting down...");
        std::lock_guard<std::mutex> lk(m_mtx);
        m_should_stop = true;
        m_cv.notify_one();
      }
    };

  } // namespace core
} // namespace daqling

#endif // DAQLING_CORE_COMMAND_HPP
