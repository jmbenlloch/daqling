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

#include "ConnectionLoader.hpp"

#include <utility>
namespace daqling {
namespace core {

bool ConnectionLoader::loadConnection(const std::string &name) {
  return load("libDaqlingConnection" + name);
}

bool ConnectionLoader::loadQueue(const std::string &name) { return load("libDaqlingQueue" + name); }

bool ConnectionLoader::load(const std::string &name) {
  std::string pluginName = name + ".so";
  m_handle = dlopen(pluginName.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (*m_handle == nullptr) {
    throw CannotOpenModule(ERS_HERE, name.c_str(), dlerror());
    m_handle.reset();
    return false;
  }
  return true;
}

std::shared_ptr<daqling::core::Sender> ConnectionLoader::getSender(const std::string &s,
                                                                   const uint &chid,
                                                                   const nlohmann::json &json,
                                                                   const std::string &datatype) {
  if (m_senderMap.find(s) == m_senderMap.end()) {
    loadConnection(s);

    if (m_senderMap.find(s) == m_senderMap.end()) {
      throw UnrecognizedConnectionType(ERS_HERE, s.c_str());
    }
  }
  ERS_DEBUG(0, "invoking lambda for sender: " << s);

  return (json.contains("queue")
              ? getQueueSender(m_senderMap[s](chid, json), chid, json["queue"], datatype)
              : m_senderMap[s](chid, json));
}
std::shared_ptr<daqling::core::Receiver>
ConnectionLoader::getReceiver(const std::string &s, const uint &chid, const nlohmann::json &json,
                              const std::string &datatype) {
  if (m_receiverMap.find(s) == m_receiverMap.end()) {
    loadConnection(s);
    if (m_receiverMap.find(s) == m_receiverMap.end()) {
      throw UnrecognizedConnectionType(ERS_HERE, s.c_str());
    }
  }
  ERS_DEBUG(0, "invoking lambda for receiver: " << s);

  return (json.contains("queue")
              ? getQueueReceiver(m_receiverMap[s](chid, json), chid, json["queue"], datatype)
              : m_receiverMap[s](chid, json));
}

std::shared_ptr<daqling::core::Sender>
ConnectionLoader::getQueueSender(std::shared_ptr<daqling::core::Sender> sen, const uint &chid,
                                 const nlohmann::json &json, const std::string &datatype) {
  auto s = json.at("type").get<std::string>();
  auto key = s + datatype;
  if (m_queueMap.find(key) == m_queueMap.end()) {
    loadQueue(s);
    if (m_queueMap.find(key) == m_queueMap.end()) {
      throw UnrecognizedQueueType(ERS_HERE, key.c_str());
    }
  }
  if (m_queuesenderMap.find(datatype) == m_queuesenderMap.end()) {
    throw UnrecognizedQueueType(ERS_HERE, datatype.c_str());
  }
  auto queueSen = m_queuesenderMap[datatype](chid, m_queueMap[s + datatype](json), std::move(sen));
  return queueSen;
}
std::shared_ptr<daqling::core::Receiver>
ConnectionLoader::getQueueReceiver(std::shared_ptr<daqling::core::Receiver> rec, const uint &chid,
                                   const nlohmann::json &json, const std::string &datatype) {
  auto s = json.at("type").get<std::string>();
  auto key = s + datatype;
  if (m_queueMap.find(key) == m_queueMap.end()) {
    loadQueue(s);
    if (m_queueMap.find(key) == m_queueMap.end()) {
      throw UnrecognizedQueueType(ERS_HERE, key.c_str());
    }
  }
  if (m_queuereceiverMap.find(datatype) == m_queuereceiverMap.end()) {
    throw UnrecognizedQueueType(ERS_HERE, datatype.c_str());
  }
  auto queueRec =
      m_queuereceiverMap[datatype](chid, m_queueMap[s + datatype](json), std::move(rec));
  return queueRec;
}
} // namespace core
} // namespace daqling