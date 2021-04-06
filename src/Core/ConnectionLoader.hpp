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

#pragma once

#include "Core/Receiver.hpp"
#include "Core/Sender.hpp"
#include "Utils/Ers.hpp"
#include "Utils/Singleton.hpp"
#include "nlohmann/json.hpp"
#include <dlfcn.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace daqling {
#include "DynamicLinkIssues.hpp"
ERS_DECLARE_ISSUE(core, ConnectionLoaderIssue, "", ERS_EMPTY)
ERS_DECLARE_ISSUE_BASE(core, UnrecognizedConnectionType, core::ConnectionLoaderIssue,
                       "Unrecognized connection type: " << type, ERS_EMPTY, ((const char *)type))
ERS_DECLARE_ISSUE_BASE(core, UnrecognizedQueueType, core::ConnectionLoaderIssue,
                       "Unrecognized queue type: " << type, ERS_EMPTY, ((const char *)type))
namespace core {

class Queue;
class Sender;
class Receiver;
class ConnectionLoader : public daqling::utilities::Singleton<ConnectionLoader> {
public:
  bool loadConnection(const std::string &name);
  bool loadQueue(const std::string &name);
  std::shared_ptr<daqling::core::Sender> getSender(const std::string & /*s*/, const uint &chid,
                                                   const nlohmann::json &json);

  std::shared_ptr<daqling::core::Receiver> getReceiver(const std::string & /*s*/, const uint &chid,
                                                       const nlohmann::json &json);
  template <class T> void addSender(const std::string &s) {
    m_senderMap[s] = [](const uint &chid, const nlohmann::json &json) {
      return std::make_shared<T>(chid, json);
    };
  }

  template <class T> void addReceiver(const std::string &s) {
    m_receiverMap[s] = [](const uint &chid, const nlohmann::json &json) {
      return std::make_shared<T>(chid, json);
    };
  }
  template <class T> void addQueue(const std::string &s) {
    m_queueMap[s] = [](const nlohmann::json &json) { return std::make_unique<T>(json); };
  }
  // static ConnectionLoader& instance();

private:
  std::shared_ptr<daqling::core::Sender>
  getQueueSender(std::shared_ptr<daqling::core::Sender> /*sen*/, const uint &chid,
                 const nlohmann::json &json);
  std::shared_ptr<daqling::core::Receiver>
  getQueueReceiver(std::shared_ptr<daqling::core::Receiver> /*rec*/, const uint &chid,
                   const nlohmann::json &json);
  bool load(const std::string &name);
  std::optional<void *> m_handle;

  std::unordered_map<std::string, std::function<std::shared_ptr<daqling::core::Sender>(
                                      const uint &, const nlohmann::json &)>>
      m_senderMap;

  std::unordered_map<std::string, std::function<std::shared_ptr<daqling::core::Receiver>(
                                      const uint &, const nlohmann::json &)>>
      m_receiverMap;

  std::unordered_map<std::string,
                     std::function<std::unique_ptr<daqling::core::Queue>(const nlohmann::json &)>>
      m_queueMap;
};
} // namespace core
} // namespace daqling