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

#include "ZMQPubSubSender.hpp"
#include "Utils/ConnectionMacros.hpp"
#include "Utils/Ers.hpp"
#include "ZMQIssues.hpp"

using namespace daqling::connection;

REGISTER_SENDER(ZMQPubSubSender, "ZMQPubSub")
ZMQPubSubSender::~ZMQPubSubSender() { m_socket->setsockopt(ZMQ_LINGER, 1); }
ZMQPubSubSender::ZMQPubSubSender(uint chid, const nlohmann::json &j) : daqling::core::Sender(chid) {
  try {
    std::string connStr;
    if (j.at("transport") == "ipc") {
      connStr = "ipc://" + j.at("path").get<std::string>();
    } else if (j.at("transport") == "tcp") {
      connStr = "tcp://" + j.at("host").get<std::string>() + ":" +
                std::to_string(j.at("port").get<uint>());
    } else {
      throw InvalidTransportType(ERS_HERE, j.at("transport").get<std::string>().c_str());
    }

    m_context = std::make_unique<zmq::context_t>(ioT);
    m_socket = std::make_unique<zmq::socket_t>(*(m_context.get()), ZMQ_PUB);

    m_socket->bind(connStr.c_str());
    ERS_INFO(" Adding PUBLISHER channel for: [" << m_chid << "] bind: " << connStr);
  } catch (ers::Issue &i) {
    throw CannotAddChannel(ERS_HERE, "caught issue", i);
  } catch (const std::exception &e) {
    throw CannotAddChannel(ERS_HERE, e.what());
  }
}
bool ZMQPubSubSender::send(const daqling::utilities::Binary &bin) {
  zmq::message_t message(bin.size());
  memcpy(message.data(), bin.data(), bin.size());
  if (m_socket->send(std::move(message))) {
    ++m_msg_handled;
    return true;
  }
  return false;
}