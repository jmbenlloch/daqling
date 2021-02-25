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

#include "ConnectionManager.hpp"
#include "Command.hpp"
#include "Utils/Binary.hpp"
#include "Utils/Ers.hpp"
#include <chrono>
#include <ctime>
#include <exception>
#include <iomanip>
#include <thread>

using namespace daqling::core;
using namespace std::chrono_literals;

bool ConnectionManager::setupStatsConnection(uint8_t ioT, std::string connStr) {
  if (m_is_stats_setup) {
    ERS_INFO(" Statistics socket is already online... Won't do anything.");
    return false;
  }
  try {
    m_stats_context = std::make_unique<zmq::context_t>(ioT);
    m_stats_socket = std::make_unique<zmq::socket_t>(*(m_stats_context.get()), ZMQ_PUB);
    m_stats_socket->connect(connStr);
    ERS_INFO(" Statistics are published on: " << connStr);
  } catch (std::exception &e) {
    throw CannotAddStatsChannel(ERS_HERE, e.what());
    return false;
  }
  m_is_stats_setup = true;
  return true;
}

bool ConnectionManager::unsetStatsConnection() {
  m_stats_socket.reset();
  m_stats_context.reset();
  m_is_stats_setup = false;
  return true;
}

bool ConnectionManager::addReceiverChannel(unsigned chn, EDirection dir, const std::string &connStr,
                                           size_t queueSize, unsigned filter, size_t filter_size) {
  // subscriber socket is an exception, as it can be connected to multiple endpoints
  if (m_receiver_sockets.find(chn) != m_receiver_sockets.end()) {
    if (dir == EDirection::SUBSCRIBER) {
      try {
        m_receiver_sockets[chn]->connect(connStr.c_str());
        ERS_INFO(" Adding SUBSCRIBER channel for: [" << chn << "] connect: " << connStr);
      } catch (std::exception &e) {
        throw CannotAddChannel(ERS_HERE, e.what());
        return false;
      }
      return true;
    } else {
      ERS_INFO(" Socket for channel already exists... Won't add this channel again.");
      return false;
    }
  }
  uint8_t ioT = 1;
  m_receiver_contexts[chn] = std::make_unique<zmq::context_t>(ioT); // Create context
  m_receiver_pcqs[chn] = std::make_unique<MessageQueue>(queueSize); // Create SPSC queue.
  m_receiver_pcqSizes[chn] = 0;       // Create stats. counter for queue
  m_receiver_numMsgsHandled[chn] = 0; //               counter for msgs
  m_receiver_directions[chn] = dir;   // Setup direction.
  try {
    if (dir == EDirection::CLIENT) {
      m_receiver_sockets[chn] =
          std::make_unique<zmq::socket_t>(*(m_receiver_contexts[chn].get()), ZMQ_PAIR);
      m_receiver_sockets[chn]->connect(connStr.c_str());
      ERS_INFO(" Adding CLIENT channel for: [" << chn << "] connect: " << connStr);
    } else if (dir == EDirection::SUBSCRIBER) {
      m_receiver_sockets[chn] =
          std::make_unique<zmq::socket_t>(*(m_receiver_contexts[chn].get()), ZMQ_SUB);
      m_receiver_sockets[chn]->connect(connStr.c_str());
      m_receiver_sockets[chn]->setsockopt(ZMQ_SUBSCRIBE, &filter, filter_size);
      ERS_INFO(" Adding SUBSCRIBER channel for: [" << chn << "] connect: " << connStr);
    }
  } catch (std::exception &e) {
    throw CannotAddChannel(ERS_HERE, e.what());
    return false;
  }
  m_receiver_channels++;
  return true;
}

bool ConnectionManager::addSenderChannel(unsigned chn, EDirection dir, const std::string &connStr,
                                         size_t queueSize) {
  uint8_t ioT = 1;
  m_sender_contexts[chn] = std::make_unique<zmq::context_t>(ioT); // Create context
  m_sender_pcqs[chn] = std::make_unique<MessageQueue>(queueSize); // Create SPSC queue.
  m_sender_pcqSizes[chn] = 0;                                     // Create stats. counter for queue
  m_sender_numMsgsHandled[chn] = 0;                               //               counter for msgs
  m_sender_directions[chn] = dir;                                 // Setup direction.
  try {
    if (dir == EDirection::SERVER) {
      m_sender_sockets[chn] =
          std::make_unique<zmq::socket_t>(*(m_sender_contexts[chn].get()), ZMQ_PAIR);
      m_sender_sockets[chn]->bind(connStr.c_str());
      ERS_INFO(" Adding SERVER channel for: [" << chn << "] bind: " << connStr);
    } else if (dir == EDirection::PUBLISHER) {
      m_sender_sockets[chn] =
          std::make_unique<zmq::socket_t>(*(m_sender_contexts[chn].get()), ZMQ_PUB);
      m_sender_sockets[chn]->bind(connStr.c_str());
      ERS_INFO(" Adding PUBLISHER channel for: [" << chn << "] bind: " << connStr);
    }
  } catch (std::exception &e) {
    throw CannotAddChannel(ERS_HERE, e.what());
    return false;
  }
  m_sender_channels++;
  return true;
}

bool ConnectionManager::removeReceiverChannel(unsigned chn) {
  m_receiver_directions.erase(chn);
  m_receiver_numMsgsHandled.erase(chn);
  m_receiver_pcqSizes.erase(chn);
  m_receiver_pcqs.erase(chn);
  m_receiver_sockets.erase(chn);
  m_receiver_contexts.erase(chn);
  m_receiver_channels--;
  return true;
}

bool ConnectionManager::removeSenderChannel(unsigned chn) {
  m_sender_directions.erase(chn);
  m_sender_numMsgsHandled.erase(chn);
  m_sender_pcqSizes.erase(chn);
  m_sender_pcqs.erase(chn);
  m_sender_sockets.erase(chn);
  m_sender_contexts.erase(chn);
  m_sender_channels--;
  return true;
}

bool ConnectionManager::addReceiveHandler(unsigned chn) {
  ERS_INFO(" [CLIENT] ReceiveHandler for channel [" << chn << "] starting...");
  m_receiver_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_receiver_sockets[chn]->recv(&msg, ZMQ_DONTWAIT)) {
        while (!m_receiver_pcqs[chn]->write(std::move(msg)) && !m_stop_handlers) {
          ERS_WARNING("Waiting queue to allow write");
          std::this_thread::sleep_for(1ms);
        }
        m_receiver_numMsgsHandled[chn]++;
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_receiver_pcqSizes[chn].store(m_receiver_pcqs[chn]->sizeGuess());
      if (m_receiver_pcqs[chn]->sizeGuess() > m_receiver_pcqs[chn]->capacity() * 0.9) {
        ERS_WARNING("CLIENT -> queue population: " << m_receiver_pcqs[chn]->sizeGuess());
      }
    }
    ERS_INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::addSendHandler(unsigned chn) {
  ERS_INFO(" [SERVER] SendHandler for channel [" << chn << "] starting...");
  m_sender_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_sender_pcqs[chn]->read(msg)) {
        m_sender_sockets[chn]->send(msg);
        m_sender_numMsgsHandled[chn]++;
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_sender_pcqSizes[chn].store(m_sender_pcqs[chn]->sizeGuess());
      if (m_sender_pcqs[chn]->sizeGuess() > m_sender_pcqs[chn]->capacity() * 0.9) {
        ERS_WARNING("SERVER -> queue population: " << m_sender_pcqs[chn]->sizeGuess());
      }
    }
    ERS_INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::addSubscribeHandler(unsigned chn) {
  ERS_INFO(" [SUB] SubscribeHandler for channel [" << chn << "] starting...");
  m_receiver_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_receiver_sockets[chn]->recv(&msg, ZMQ_DONTWAIT)) {
        while (!m_receiver_pcqs[chn]->write(std::move(msg)) && !m_stop_handlers) {
          ERS_WARNING("Waiting queue to allow write");
          std::this_thread::sleep_for(1ms);
        }
        m_receiver_numMsgsHandled[chn]++;
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_receiver_pcqSizes[chn].store(m_receiver_pcqs[chn]->sizeGuess());
      if (m_receiver_pcqs[chn]->sizeGuess() > m_receiver_pcqs[chn]->capacity() * 0.9) {
        ERS_WARNING("SUB -> queue population: " << m_receiver_pcqs[chn]->sizeGuess());
      }
    }
    ERS_INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::addPublishHandler(unsigned chn) {
  ERS_INFO(" [PUB] PublishHandler for channel [" << chn << "] starting...");
  m_sender_handlers[chn] = std::thread([&, chn]() {
    while (!m_stop_handlers) {
      zmq::message_t msg;
      if (m_sender_pcqs[chn]->read(msg)) {
        m_sender_sockets[chn]->send(msg);
        m_sender_numMsgsHandled[chn]++;
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_sender_pcqSizes[chn].store(m_sender_pcqs[chn]->sizeGuess());
      if (m_sender_pcqs[chn]->sizeGuess() > m_sender_pcqs[chn]->capacity() * 0.9) {
        ERS_WARNING("PUB -> queue population: " << m_sender_pcqs[chn]->sizeGuess());
      }
    }
    ERS_INFO(" joining channel [" << chn << "] handler.");
  });
  return true;
}

bool ConnectionManager::receive(const unsigned &chn, daqling::utilities::Binary &bin) {
  if (m_receiver_pcqs[chn]->sizeGuess() != 0) {
    utilities::Binary msgBin(m_receiver_pcqs[chn]->frontPtr()->data(),
                             m_receiver_pcqs[chn]->frontPtr()->size());
    m_receiver_pcqs[chn]->popFront();
    bin = msgBin;
    return true;
  }
  return false;
}

bool ConnectionManager::send(const unsigned &chn, const daqling::utilities::Binary &msgBin) {
  zmq::message_t message(msgBin.size());
  memcpy(message.data(), msgBin.data(), msgBin.size());
  return m_sender_pcqs[chn]->write(std::move(message));
}

bool ConnectionManager::start() {
  m_stop_handlers.store(false);
  for (auto const &dirIt : m_receiver_directions) //([first: chn, second:dir])
  {
    switch (dirIt.second) {
    case CLIENT:
      addReceiveHandler(dirIt.first);
      break;
    case SUBSCRIBER:
      addSubscribeHandler(dirIt.first);
      break;
    default:
      return false;
    }
  }
  for (auto const &dirIt : m_sender_directions) //([first: chn, second:dir])
  {
    switch (dirIt.second) {
    case SERVER:
      addSendHandler(dirIt.first);
      break;
    case PUBLISHER:
      addPublishHandler(dirIt.first);
      break;
    default:
      return false;
    }
  }
  return true; // TODO put some meaning or return void
}

bool ConnectionManager::stop() {
  m_stop_handlers.store(true);
  for (auto &tIt : m_receiver_handlers) {
    tIt.second.join();
  }
  for (auto &tIt : m_sender_handlers) {
    tIt.second.join();
  }
  m_receiver_handlers.clear();
  m_sender_handlers.clear();
  return true; // TODO put some meaning or return void
}
