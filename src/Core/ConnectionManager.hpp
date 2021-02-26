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

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "Command.hpp"
#include "Utils/Binary.hpp"
#include "Utils/Ers.hpp"
#include "Utils/ProducerConsumerQueue.hpp"
#include <zmq.hpp>
#define MSGQ

namespace daqling {
#include <ers/Issue.h>

ERS_DECLARE_ISSUE(core, ConnectionIssue, "", ERS_EMPTY)

ERS_DECLARE_ISSUE_BASE(core, CannotAddChannel, core::ConnectionIssue,
                       "Failed to add channel! ZMQ returned: " << eWhat, ERS_EMPTY,
                       ((const char *)eWhat))

ERS_DECLARE_ISSUE_BASE(core, CannotAddStatsChannel, core::CannotAddChannel,
                       "Failed to add stats channel! ZMQ returned: " << eWhat,
                       ((const char *)eWhat), ERS_EMPTY)
namespace core {

/*
 * ConnectionManager
 * Description: Wrapper class for sockets and SPSC circular buffers.
 *   Makes the communication between DAQ processes easier and scalable.
 * Date: November 2017
 */

// template <class CT, class ST>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class ConnectionManager : public daqling::utilities::Singleton<ConnectionManager> {
public:
  ConnectionManager()
      : m_receiver_channels{0}, m_sender_channels{0}, m_is_stats_setup{false}, m_stop_handlers{
                                                                                   false} {}
  ~ConnectionManager() { m_stop_handlers = true; }

  // Custom types
  using MessageQueue = folly::ProducerConsumerQueue<zmq::message_t>;
  using UniqueMessageQueue = std::unique_ptr<MessageQueue>;
  using StringQueue = folly::ProducerConsumerQueue<std::string>;
  using UniqueStringQueue = std::unique_ptr<StringQueue>;
  typedef std::map<unsigned, std::atomic<size_t>> SizeStatMap;

  // Enums
  enum EDirection { SERVER, CLIENT, PUBLISHER, SUBSCRIBER };

  // Functionalities
  bool setupStatsConnection(uint8_t ioT, const std::string &connStr);
  bool unsetStatsConnection();

  // Add a channel (sockets and queues)
  bool addReceiverChannel(unsigned chn, EDirection dir, const std::string &connStr,
                          size_t queueSize, unsigned filter = 0, size_t filter_size = 0);
  bool addSenderChannel(unsigned chn, EDirection dir, const std::string &connStr, size_t queueSize);

  bool removeReceiverChannel(unsigned chn);
  bool removeSenderChannel(unsigned chn);

  /**
   * @brief Receive binary from receiver channel
   * @param chn receiver channel id
   * @return true when binary file is successfully passed
   */
  bool receive(const unsigned &chn, daqling::utilities::Binary &bin);
  /**
   * @brief Send binary to channel
   * @param chn sender channel id
   * @return true when binary file is successfully passed
   */
  bool send(const unsigned &chn, const daqling::utilities::Binary &msgBin);

  // Start/stop socket processors
  bool start();
  bool stop();

  // Utilities
  unsigned getNumOfReceiverChannels() { return m_receiver_channels; }
  unsigned getNumOfSenderChannels() { return m_sender_channels; }
  std::atomic<size_t> &getReceiverQueueStat(unsigned chn) { return m_receiver_pcqSizes[chn]; }
  std::atomic<size_t> &getSenderQueueStat(unsigned chn) { return m_sender_pcqSizes[chn]; }
  std::atomic<size_t> &getReceiverMsgStat(unsigned chn) { return m_receiver_numMsgsHandled[chn]; }
  std::atomic<size_t> &getSenderMsgStat(unsigned chn) { return m_sender_numMsgsHandled[chn]; }
  const SizeStatMap &getReceiverStatsMap() { return std::ref(m_receiver_pcqSizes); }
  const SizeStatMap &getSenderStatsMap() { return std::ref(m_sender_pcqSizes); }
  const SizeStatMap &getReceiverMsgStatsMap() { return std::ref(m_receiver_numMsgsHandled); }
  const SizeStatMap &getSenderMsgStatsMap() { return std::ref(m_sender_numMsgsHandled); }
  std::unique_ptr<zmq::socket_t> &getStatSocket() { return std::ref(m_stats_socket); }

private:
  const std::string m_className = "ConnectionManager";
  size_t m_receiver_channels;
  size_t m_sender_channels;

  std::map<unsigned, UniqueMessageQueue> m_receiver_pcqs; // Queues for elink RX.
  std::map<unsigned, UniqueMessageQueue> m_sender_pcqs;   // Queues for elink TX.

  // Stats
  SizeStatMap m_receiver_pcqSizes;
  SizeStatMap m_sender_pcqSizes;
  SizeStatMap m_receiver_numMsgsHandled;
  SizeStatMap m_sender_numMsgsHandled;

  // Network library handling
  // Statistics
  std::unique_ptr<zmq::context_t> m_stats_context;
  std::unique_ptr<zmq::socket_t> m_stats_socket;
  std::atomic<bool> m_is_stats_setup;
  // Dataflow
  std::map<unsigned, std::unique_ptr<zmq::context_t>> m_receiver_contexts; // context descriptors
  std::map<unsigned, std::unique_ptr<zmq::context_t>> m_sender_contexts;   // context descriptors

  std::map<unsigned, std::unique_ptr<zmq::socket_t>> m_receiver_sockets; // sockets.
  std::map<unsigned, std::unique_ptr<zmq::socket_t>> m_sender_sockets;   // sockets.

  std::map<unsigned, EDirection> m_receiver_directions;
  std::map<unsigned, EDirection> m_sender_directions;

  // Threads
  std::map<unsigned, std::thread> m_receiver_handlers;
  std::map<unsigned, std::thread> m_sender_handlers;
  std::map<unsigned, std::unique_ptr<daqling::utilities::ReusableThread>> m_processors;
  std::map<unsigned, std::function<void()>> m_functors;

  // Thread control
  std::atomic<bool> m_stop_handlers;

  std::mutex m_mutex;
  std::mutex m_mtx_cleaning;

  // Internal
  bool addSendHandler(unsigned chn); // std::function<void()> task);
  bool addReceiveHandler(unsigned chn);
  bool addPublishHandler(unsigned chn);
  bool addSubscribeHandler(unsigned chn);
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_CONNECTIONMANAGER_HPP
