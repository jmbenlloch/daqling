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
#include <exception>
#include <iomanip>
#include <thread>
/// \endcond

#include "Command.hpp"
#include "ConnectionManager.hpp"
#include "Utils/Binary.hpp"
#include "Utils/Logging.hpp"

using namespace daqling::core;
using namespace std::chrono_literals;

bool ConnectionManager::setupCommandConnection(uint8_t ioT, std::string connStr) {
  if (m_is_cmd_setup) {
    INFO(" Command is already online... Won't do anything.");
    return false;
  }
  try {
    m_cmd_context = std::make_unique<zmq::context_t>(ioT);
    m_cmd_socket = std::make_unique<zmq::socket_t>(*(m_cmd_context.get()), ZMQ_REP);
    m_cmd_socket->bind(connStr);
    INFO(" Command is connected on: " << connStr);
  } catch (std::exception &e) {
    ERROR(" Failed to add Command channel! ZMQ returned: " << e.what());
    return false;
  }
  m_cmd_handler = std::thread([&]() {
    Command &cmd = Command::instance();
    zmq::message_t cmdMsg;
    while (!m_stop_cmd_handler) {
      // INFO(m_className << " CMD_THREAD: Going for RECV poll...");
      if (m_cmd_socket->recv(&cmdMsg, ZMQ_DONTWAIT)) {
        const std::string cmdStr(static_cast<char *>(cmdMsg.data()), cmdMsg.size());
        DEBUG(m_className << " CMD_THREAD: Got CMD: " << cmdStr);
        cmd.setCommand(cmdStr);
        int more;
        size_t more_size = sizeof(int);
        m_cmd_socket->getsockopt(ZMQ_RCVMORE, &more, &more_size);
        DEBUG("getsockopt RCVMORE " << more);
        if (more) {
          zmq::message_t configMsg;
          m_cmd_socket->recv(&configMsg, ZMQ_DONTWAIT);
          const std::string argumentStr(static_cast<char *>(configMsg.data()), configMsg.size());
          DEBUG("======== RECEIVE MORE " << argumentStr);
          cmd.setArgument(argumentStr);
        }

        cmd.handleCommand();
        s_send(*(m_cmd_socket.get()), cmd.getResponse());
      } else {
        // INFO(m_className << " Sleeping a second...");
        std::this_thread::sleep_for(1ms);
      }
    }
  });
  utilities::setThreadName(m_cmd_handler, "cmd", 0);
  m_is_cmd_setup = true;
  return true;
}

bool ConnectionManager::setupStatsConnection(uint8_t ioT, std::string connStr) {
  if (m_is_stats_setup) {
    INFO(" Statistics socket is already online... Won't do anything.");
    return false;
  }
  try {
    m_stats_context = std::make_unique<zmq::context_t>(ioT);
    m_stats_socket = std::make_unique<zmq::socket_t>(*(m_stats_context.get()), ZMQ_PUB);
    m_stats_socket->bind(connStr);
    INFO(" Statistics are published on: " << connStr);
  } catch (std::exception &e) {
    ERROR(" Failed to add Stats publisher channel! ZMQ returned: " << e.what());
    return false;
  }
  return true;
}

bool ConnectionManager::addChannel(uint64_t chn, EDirection dir, const std::string &connStr,
                                   size_t queueSize) {
  // subscriber socket is an exception, as it can be connected to multiple endpoints
  if (m_sockets.find(chn) != m_sockets.end()) {
    if (dir == EDirection::SUBSCRIBER) {
      try {
        m_sockets[chn]->connect(connStr.c_str());
        INFO(" Adding SUBSCRIBER channel for: [" << chn << "] connect: " << connStr);
      } catch (std::exception &e) {
        ERROR(" Failed to add channel! ZMQ returned: " << e.what());
        return false;
      }
      return true;
    } else {
      INFO(" Socket for channel already exists... Won't add this channel again.");
      return false;
    }
  }
  uint8_t ioT = 1;
  m_contexts[chn] = std::make_unique<zmq::context_t>(ioT); // Create context
  m_pcqs[chn] = std::make_unique<MessageQueue>(queueSize); // Create SPSC queue.
  m_pcqSizes[chn] = {0};                                   // Create stats. counter for queue
  m_numMsgsHandled[chn] = {0};                             //               counter for msgs
  m_directions[chn] = dir;                                 // Setup direction.
  try {
    if (dir == EDirection::SERVER) {
      m_sockets[chn] = std::make_unique<zmq::socket_t>(*(m_contexts[chn].get()), ZMQ_PAIR);
      m_sockets[chn]->bind(connStr.c_str());
      INFO(" Adding SERVER channel for: [" << chn << "] bind: " << connStr);
    } else if (dir == EDirection::CLIENT) {
      m_sockets[chn] = std::make_unique<zmq::socket_t>(*(m_contexts[chn].get()), ZMQ_PAIR);
      m_sockets[chn]->connect(connStr.c_str());
      INFO(" Adding CLIENT channel for: [" << chn << "] connect: " << connStr);
    } else if (dir == EDirection::PUBLISHER) {
      m_sockets[chn] = std::make_unique<zmq::socket_t>(*(m_contexts[chn].get()), ZMQ_PUB);
      m_sockets[chn]->bind(connStr.c_str());
      INFO(" Adding PUBLISHER channel for: [" << chn << "] bind: " << connStr);
    } else if (dir == EDirection::SUBSCRIBER) {
      m_sockets[chn] = std::make_unique<zmq::socket_t>(*(m_contexts[chn].get()), ZMQ_SUB);
      m_sockets[chn]->connect(connStr.c_str());
      m_sockets[chn]->setsockopt(ZMQ_SUBSCRIBE, "", 0); // TODO add a tag?
      INFO(" Adding SUBSCRIBER channel for: [" << chn << "] connect: " << connStr);
    }
  } catch (std::exception &e) {
    ERROR(" Failed to add channel! ZMQ returned: " << e.what());
    return false;
  }
  return true;
}

bool ConnectionManager::addReceiveHandler(uint64_t chn) {
  INFO(" [CLIENT] ReceiveHandler for channel [" << chn << "] starting...");
  m_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_sockets[chn]->recv(&msg, ZMQ_DONTWAIT)) {
        m_pcqs[chn]->write(std::move(msg));
        m_numMsgsHandled[chn]++;
        // DEBUG("    -> wrote to queue");
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_pcqSizes[chn].store(m_pcqs[chn]->sizeGuess());
      if (m_pcqs[chn]->sizeGuess() > m_pcqs[chn]->capacity() * 0.9) {
        WARNING("CLIENT -> queue population: " << m_pcqs[chn]->sizeGuess());
      }
    }
    INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::addSendHandler(uint64_t chn) {
  INFO(" [SERVER] SendHandler for channel [" << chn << "] starting...");
  m_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_pcqs[chn]->read(msg)) {
        m_sockets[chn]->send(msg);
        m_numMsgsHandled[chn]++;
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_pcqSizes[chn].store(m_pcqs[chn]->sizeGuess());
      if (m_pcqs[chn]->sizeGuess() > m_pcqs[chn]->capacity() * 0.9) {
        WARNING("SERVER -> queue population: " << m_pcqs[chn]->sizeGuess());
      }
    }
    INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::addSubscribeHandler(uint64_t chn) {
  INFO(" [SUB] SubscribeHandler for channel [" << chn << "] starting...");
  m_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_sockets[chn]->recv(&msg, ZMQ_DONTWAIT)) {
        m_pcqs[chn]->write(std::move(msg));
        m_numMsgsHandled[chn]++;
        // DEBUG("    -> wrote to queue");
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_pcqSizes[chn].store(m_pcqs[chn]->sizeGuess());
      if (m_pcqs[chn]->sizeGuess() > m_pcqs[chn]->capacity() * 0.9) {
        WARNING("SUB -> queue population: " << m_pcqs[chn]->sizeGuess());
      }
    }
    INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::addPublishHandler(uint64_t chn) {
  INFO(" [PUB] PublishHandler for channel [" << chn << "] starting...");
  m_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_pcqs[chn]->read(msg)) {
        m_sockets[chn]->send(msg);
        m_numMsgsHandled[chn]++;
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_pcqSizes[chn].store(m_pcqs[chn]->sizeGuess());
      if (m_pcqs[chn]->sizeGuess() > m_pcqs[chn]->capacity() * 0.9) {
        WARNING("PUB -> queue population: " << m_pcqs[chn]->sizeGuess());
      }
    }
    INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::get(uint64_t chn, daqling::utilities::Binary &bin) {
  if (m_pcqs[chn]->sizeGuess() != 0) {
    utilities::Binary msgBin(m_pcqs[chn]->frontPtr()->data(), m_pcqs[chn]->frontPtr()->size());
    m_pcqs[chn]->popFront();
    bin = msgBin;
    return true;
  }
  return false;
}

void ConnectionManager::put(uint64_t chn, daqling::utilities::Binary &msgBin) {
  zmq::message_t message(msgBin.size());
  memcpy(message.data(), msgBin.data(), msgBin.size());
  m_pcqs[chn]->write(std::move(message));
}

void ConnectionManager::putStr(uint64_t chn, const std::string &string) {
  zmq::message_t message(string.size());
  memcpy(message.data(), string.data(), string.size());
  m_pcqs[chn]->write(std::move(message));
}

std::string ConnectionManager::getStr(uint64_t chn) {
  std::string s("");
  if (m_pcqs[chn]->sizeGuess() != 0) {
    s = std::string(static_cast<char *>(m_pcqs[chn]->frontPtr()->data()),
                    m_pcqs[chn]->frontPtr()->size());
    m_pcqs[chn]->popFront();
  }
  return s;
}

bool ConnectionManager::start() {
  m_stop_handlers.store(false);
  for (auto const &dirIt : m_directions) //([first: chn, second:dir])
  {
    switch (dirIt.second) {
    case CLIENT:
      addReceiveHandler(dirIt.first);
      break;
    case SERVER:
      addSendHandler(dirIt.first);
      break;
    case PUBLISHER:
      addPublishHandler(dirIt.first);
      break;
    case SUBSCRIBER:
      addSubscribeHandler(dirIt.first);
      break;
    }
  }
  return true; // TODO put some meaning or return void
}

bool ConnectionManager::stop() {
  m_stop_handlers.store(true);
  std::this_thread::sleep_for(100ms); // allow time to stop
  for (auto &tIt : m_handlers) {
    tIt.second.join();
  }
  m_handlers.clear();
  return true; // TODO put some meaning or return void
}
