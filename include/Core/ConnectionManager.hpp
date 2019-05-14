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

#ifndef DAQLING_CORE_CONNECTIONMANAGER_HPP
#define DAQLING_CORE_CONNECTIONMANAGER_HPP

/// \cond
#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include "Utilities/zhelpers.hpp"
/// \endcond

#include "Core/Command.hpp"
#include "Utilities/Binary.hpp"
#include "Utilities/ProducerConsumerQueue.hpp"
#include "Utilities/ReusableThread.hpp"
#include "Utilities/Singleton.hpp"

#define MSGQ

namespace daqling {
namespace core {

/*
 * ConnectionManager
 * Author: Roland.Sipos@cern.ch
 * Description: Wrapper class for sockets and SPSC circular buffers.
 *   Makes the communication between DAQ processes easier and scalable.
 * Date: November 2017
 */

// template <class CT, class ST>
class ConnectionManager : public daqling::utilities::Singleton<ConnectionManager> {
 public:
  //
  ConnectionManager() : m_is_cmd_setup{false}, m_stop_cmd_handler{false}, m_stop_handlers{false} {}
  ~ConnectionManager() {
    m_stop_handlers = true;
    m_stop_cmd_handler = true;
    m_cmd_handler.join();
  }


  // Custom types
  typedef folly::ProducerConsumerQueue<zmq::message_t> MessageQueue;
  typedef std::unique_ptr<MessageQueue> UniqueMessageQueue;
  typedef folly::ProducerConsumerQueue<std::string> StringQueue;
  typedef std::unique_ptr<StringQueue> UniqueStringQueue;

  // Enums
  enum EDirection { SERVER, CLIENT, PUBLISHER, SUBSCRIBER };

  // Functionalities
  bool setupCommandConnection(uint8_t ioT, std::string connStr);

  // Add a channel (sockets and queues)
  bool addChannel(uint64_t chn, EDirection dir, uint16_t tag, std::string host, uint16_t port,
                  size_t queueSize, bool zerocopy);
  bool addChannel(uint64_t chn, EDirection dir, const std::string& connStr, size_t queueSize);

  // Getter/Putter for channels:
  /**
  * @brief Get binary from channel
  * 
  * @return true when binary file is successfully passed
  */
  bool get(uint64_t chn, daqling::utilities::Binary& bin);
  void put(uint64_t chn, daqling::utilities::Binary& msgBin);
  void putStr(uint64_t chn, const std::string& string);
  std::string getStr(uint64_t chn);

  // Start/stop socket processors
  bool start();
  bool stop();

  // Utilities
  size_t getNumOfChannels() { return m_activeChannels; }  // Get the number of active channels.

  /*
    bool connect(uint64_t chn, uint16_t tag) { return false; } // Connect/subscriber to given
    channel. bool disconnect(uint64_t chn, uint16_t tag) { return false; } //
    Disconnect/unsubscriber from a given channel. void start() {} // Starts the subscri threads.
    void stopSubscribers() {}  // Stops the subscriber threads.
    bool busy() { return false; } // are processor threads busy
  */

 private:
  const std::string m_className = "ConnectionManager";
  size_t m_activeChannels;

  // Configuration:
  std::vector<uint64_t> m_channels;

  // Queues
#ifdef MSGQ
  std::map<uint64_t, UniqueMessageQueue> m_pcqs;  // Queues for elink RX.
#else
  // std::map<uint64_t, UniqueFrameQueue> m_pcqs;
#endif

  // Network library handling
  std::thread m_cmd_handler;
  std::unique_ptr<zmq::context_t> m_cmd_context;
  std::unique_ptr<zmq::socket_t> m_cmd_socket;
  std::atomic<bool> m_is_cmd_setup;

  std::map<uint64_t, std::unique_ptr<zmq::context_t>> m_contexts;  // context descriptors
  std::map<uint64_t, std::unique_ptr<zmq::socket_t>> m_sockets;    // sockets.
  std::map<uint64_t, EDirection> m_directions;

  // Threads
  std::map<uint64_t, std::thread> m_handlers;
  std::map<uint64_t, std::unique_ptr<daqling::utilities::ReusableThread>> m_processors;
  std::map<uint64_t, std::function<void()>> m_functors;

  // Thread control
  std::atomic<bool> m_stop_cmd_handler;
  std::atomic<bool> m_stop_handlers;
  std::atomic<bool> m_stop_processors;
  std::atomic<bool> m_cpu_lock;

  std::mutex m_mutex;
  std::mutex m_mtx_cleaning;

  // Internal
  bool addSendHandler(uint64_t chn);  // std::function<void()> task);
  bool addReceiveHandler(uint64_t chn);
  bool addPublishHandler(uint64_t chn);
  bool addSubscribeHandler(uint64_t chn);

};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_CONNECTIONMANAGER_HPP
