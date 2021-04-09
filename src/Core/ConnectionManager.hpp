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

#ifndef DAQLING_CORE_CONNECTIONMANAGER_HPP
#define DAQLING_CORE_CONNECTIONMANAGER_HPP

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

#include "Command.hpp"
#include "Core/Receiver.hpp"
#include "Core/Sender.hpp"
#include "Utils/Binary.hpp"
#include "Utils/Ers.hpp"
#include "nlohmann/json.hpp"
#include <zmq.hpp>

namespace daqling {

// change to be more general, e.g. couldn't add connection with name bla. bla. -> cause
ERS_DECLARE_ISSUE(core, ConnectionIssue, "", ERS_EMPTY)

ERS_DECLARE_ISSUE_BASE(core, CannotAddChannel, core::ConnectionIssue, "Failed to add channel!",
                       ERS_EMPTY, ERS_EMPTY)
ERS_DECLARE_ISSUE_BASE(core, CannotGetChidAndType, core::ConnectionIssue,
                       "Failed to get chid and connection type " << eWhat, ERS_EMPTY,
                       ((const char *)eWhat))
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
  ConnectionManager() : m_receiver_channels{0}, m_sender_channels{0} {}
  // Custom types
  using SizeStatMap = std::map<unsigned, std::atomic<size_t>>;
  using SenderMap = std::map<unsigned, std::shared_ptr<Sender>>;
  using ReceiverMap = std::map<unsigned, std::shared_ptr<Receiver>>;

  // Add a channel (sockets and queues)
  bool removeReceiverChannel(unsigned chn);
  bool removeSenderChannel(unsigned chn);
  bool addSenderChannel(const nlohmann::json &j);
  bool addReceiverChannel(const nlohmann::json &j);
  /**
   * @brief Receive binary from receiver channel
   * @param chn receiver channel id
   * @return true when binary file is successfully passed
   */
  bool receive(const unsigned &chn, daqling::utilities::Binary &bin);
  bool sleep_receive(const unsigned &chn, daqling::utilities::Binary &bin);
  /**
   * @brief Send binary to channel
   * @param chn sender channel id
   * @return true when binary file is successfully passed
   */
  bool send(const unsigned &chn, const daqling::utilities::Binary &msgBin);
  bool sleep_send(const unsigned &chn, const daqling::utilities::Binary &msgBin);

  /**
   * @brief Set sleep duration for receiver
   * @param chn receiver channel id
   * @param ms delay in ms
   */
  void set_receiver_sleep_duration(const unsigned &chn, uint ms);
  /**
   * @brief Set sleep duration for sender
   * @param chn sender channel id
   * @param ms delay in ms
   */
  void set_sender_sleep_duration(const unsigned &chn, uint ms);
  // Start/stop socket processors
  bool start();
  bool stop();

  // Utilities
  unsigned getNumOfReceiverChannels() { return m_receiver_channels; }
  unsigned getNumOfSenderChannels() { return m_sender_channels; }
  std::atomic<size_t> &getReceiverQueueStat(unsigned chn) { return m_receivers[chn]->getPcqSize(); }
  std::atomic<size_t> &getSenderQueueStat(unsigned chn) { return m_senders[chn]->getPcqSize(); }
  std::atomic<size_t> &getReceiverMsgStat(unsigned chn) {
    return m_receivers[chn]->getMsgsHandled();
  }
  std::atomic<size_t> &getSenderMsgStat(unsigned chn) { return m_senders[chn]->getMsgsHandled(); }

private:
  size_t m_receiver_channels;
  size_t m_sender_channels;
  SenderMap m_senders;
  ReceiverMap m_receivers;

  std::mutex m_mutex;
  std::mutex m_mtx_cleaning;
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_CONNECTIONMANAGER_HPP
