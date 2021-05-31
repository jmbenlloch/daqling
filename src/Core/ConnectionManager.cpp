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

#include "ConnectionManager.hpp"

#include "ConnectionLoader.hpp"
#include "Utils/Binary.hpp"
#include "Utils/Ers.hpp"
#include <utility>

using namespace daqling::core;

bool ConnectionManager::addReceiverChannel(const nlohmann::json &j, const std::string &datatype) {
  auto &cl = daqling::core::ConnectionLoader::instance();
  std::string type;
  uint chid;
  try {
    chid = j.at("chid").get<uint>();
    type = j.at("type").get<std::string>();
  } catch (const std::exception &e) {
    throw CannotGetChidAndType(ERS_HERE, e.what());
    return false;
  }

  if (m_receivers.find(chid) != m_receivers.end()) {
    ERS_INFO(" Socket for channel already exists... Won't add this channel again.");
    return false;
  }

  try {
    m_receivers[chid] = cl.getReceiver(type, chid, j, datatype);
    ERS_INFO(" Adding RECEIVER channel for: [" << chid << "] type: " << type);
  } catch (ers::Issue &i) {
    throw CannotAddChannel(ERS_HERE, "Caught Issue", i);
    return false;
  }
  m_receivers[chid]->set_sleep_duration(1);
  m_receiver_channels++;
  return true;
}

bool ConnectionManager::addSenderChannel(const nlohmann::json &j, const std::string &datatype) {
  auto &cl = daqling::core::ConnectionLoader::instance();
  uint chid;
  std::string type;
  try {
    chid = j.at("chid").get<uint>();
    type = j.at("type").get<std::string>();
  } catch (const std::exception &e) {
    throw CannotGetChidAndType(ERS_HERE, e.what());
    return false;
  }

  if (m_senders.find(chid) != m_senders.end()) {
    ERS_INFO(" Socket for channel already exists... Won't add this channel again.");
    return false;
  }

  try {
    m_senders[chid] = cl.getSender(type, chid, j, datatype);
    ERS_INFO(" Adding SENDER channel for: [" << chid << "] type: " << type);
  } catch (ers::Issue &i) {
    throw CannotAddChannel(ERS_HERE, "Caught issue", i);
    return false;
  }
  m_senders[chid]->set_sleep_duration(1);
  m_sender_channels++;
  return true;
}

bool ConnectionManager::removeReceiverChannel(unsigned chn) {
  m_receivers.erase(chn);
  m_receiver_channels--;
  return true;
}

bool ConnectionManager::removeSenderChannel(unsigned chn) {
  m_senders.erase(chn);
  m_sender_channels--;
  return true;
}

bool ConnectionManager::receive(const unsigned &chn, DataType &bin) {
  return m_receivers[chn]->receive(bin);
}
bool ConnectionManager::sleep_receive(const unsigned &chn, DataType &bin) {
  return m_receivers[chn]->sleep_receive(bin);
}

bool ConnectionManager::send(const unsigned &chn, DataType &msgBin) {
  return m_senders[chn]->send(msgBin);
}
bool ConnectionManager::sleep_send(const unsigned &chn, DataType &msgBin) {
  return m_senders[chn]->sleep_send(msgBin);
}

void ConnectionManager::set_receiver_sleep_duration(const unsigned &chn, uint ms) {
  m_receivers[chn]->set_sleep_duration(ms);
}
void ConnectionManager::set_sender_sleep_duration(const unsigned &chn, uint ms) {
  m_senders[chn]->set_sleep_duration(ms);
}
bool ConnectionManager::start() {
  for (auto const &it : m_receivers) //([first: chn, second:dir])
  {
    it.second->start();
  }
  for (auto const &it : m_senders) //([first: chn, second:dir])
  {

    it.second->start();
  }
  return true; // TODO: put some meaning or return void
}

bool ConnectionManager::stop() {
  for (auto const &it : m_receivers) //([first: chn, second:dir])
  {
    it.second->stop();
  }
  for (auto const &it : m_senders) //([first: chn, second:dir])
  {
    it.second->stop();
  }
  return true; // TODO: put some meaning or return void
}
