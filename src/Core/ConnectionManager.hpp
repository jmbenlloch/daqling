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

#include <map>
#include <mutex>

#include "Core/Receiver.hpp"
#include "Core/Sender.hpp"
#include "ResourceFactory.hpp"
#include "Utils/Binary.hpp"
#include "Utils/Ers.hpp"
#include "Utils/Singleton.hpp"
#include "nlohmann/json.hpp"
namespace daqling {

// change to be more general, e.g. couldn't add connection with name bla. bla. -> cause
ERS_DECLARE_ISSUE(core, ConnectionIssue, "", ERS_EMPTY)

ERS_DECLARE_ISSUE_BASE(core, CannotAddChannel, core::ConnectionIssue, "Failed to add channel!",
                       ERS_EMPTY, ERS_EMPTY)
ERS_DECLARE_ISSUE_BASE(core, CannotGetChidAndType, core::ConnectionIssue,
                       "Failed to get chid and connection type " << eWhat, ERS_EMPTY,
                       ((const char *)eWhat))
ERS_DECLARE_ISSUE_BASE(core, CannotGetSubManager, core::ConnectionIssue,
                       "Failed to get submanager with number: " << no, ERS_EMPTY, ((uint)no))
namespace core {

// forward declare ConnectionSubManager
class ConnectionSubManager;

/*
 * ConnectionManager
 * Description: Manager responsible for storing ConnectionSubManagers for all modules in context,
 * as well as creating the connections these need, based on configs.
 * Date: November 2017
 */

// template <class CT, class ST>
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class ConnectionManager : public daqling::utilities::Singleton<ConnectionManager> {
public:
  ConnectionManager() : m_receiver_channels{0}, m_sender_channels{0} {}
  // Custom types
  using SizeStatMap = std::map<unsigned, std::atomic<size_t>>;
  using SubManagerMap = std::map<std::string, std::shared_ptr<ConnectionSubManager>>;
  // Teardown
  bool removeChannel(const std::string &name);
  // Add a channel (sockets and queues)
  bool addSenderChannel(const std::string & /*key*/, const nlohmann::json &j,
                        const std::string &datatype);
  bool addReceiverChannel(const std::string & /*key*/, const nlohmann::json &j,
                          const std::string &datatype);

  // Utilities
  ConnectionSubManager &addSubManager(std::string key);

  // Start/stop socket processors
  bool start(const std::string &name);
  bool stop(const std::string &name);

  std::shared_ptr<daqling::utilities::Resource> getLocalResource(unsigned id);

  void addLocalResource(const nlohmann::json &json);

private:
  // Map of ConnectionSubManagers
  SubManagerMap m_sub_managers;
  size_t m_receiver_channels;
  size_t m_sender_channels;

  std::mutex m_mutex;
  std::mutex m_mtx_cleaning;
};

/*
 * ConnectionSubManager
 * Description: Wrapper class for sockets and SPSC circular buffers.
 *   Makes the communication between DAQ processes easier and scalable.
 * Date: May 2021
 */
class ConnectionSubManager {
  friend class ConnectionManager;

public:
  using SenderMap = std::map<unsigned, std::shared_ptr<Sender>>;
  using ReceiverMap = std::map<unsigned, std::shared_ptr<Receiver>>;

  ConnectionSubManager(std::string name);
  /**
   * @brief Receive binary from receiver channel
   * @param chn receiver channel id
   * @return true when binary file is successfully passed
   */
  bool receive(const unsigned &chn, DataType &bin);
  bool sleep_receive(const unsigned &chn, DataType &bin);
  /**
   * @brief Send binary to channel
   * @param chn sender channel id
   * @return true when binary file is successfully passed
   */
  bool send(const unsigned &chn, DataType &msgBin);
  bool sleep_send(const unsigned &chn, DataType &msgBin);

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

  bool removeReceiverChannel(unsigned chn);
  bool removeSenderChannel(unsigned chn);
  // Utilities
  const SenderMap getSenderMap() { return m_senders; }
  const ReceiverMap getReceiverMap() { return m_receivers; }
  const std::string getType() { return m_type; }

private:
  SenderMap m_senders;
  ReceiverMap m_receivers;
  size_t m_receiver_channels{};
  size_t m_sender_channels{};
  std::string m_name;
  std::string m_type;
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_CONNECTIONMANAGER_HPP
