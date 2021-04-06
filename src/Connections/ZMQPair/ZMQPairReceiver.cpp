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

#include "ZMQPairReceiver.hpp"
#include "Utils/ConnectionMacros.hpp"
#include "Utils/Ers.hpp"
#include "ZMQIssues.hpp"
#include <iomanip>

using namespace daqling::connection;

REGISTER_RECEIVER(ZMQPairReceiver, "ZMQPair")

ZMQPairReceiver::ZMQPairReceiver(uint chid, const nlohmann::json &j)
    : daqling::core::Receiver(chid) {
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
    m_socket = std::make_unique<zmq::socket_t>(*(m_context.get()), ZMQ_PAIR);
    m_socket->connect(connStr.c_str());

    ERS_INFO(" Adding CLIENT channel for: [" << m_chid << "] connect: " << connStr);
  } catch (ers::Issue &i) {
    throw CannotAddChannel(ERS_HERE, "caught issue", i);
  } catch (const std::exception &e) {
    throw CannotAddChannel(ERS_HERE, e.what());
  }
}
void ZMQPairReceiver::set_sleep_duration(uint ms) { m_socket->setsockopt(ZMQ_RCVTIMEO, ms); }
bool ZMQPairReceiver::receive(daqling::utilities::Binary &bin) {
  zmq::message_t msg;
  if (m_socket->recv(&msg, ZMQ_DONTWAIT)) {
    utilities::Binary msgBin(msg.data(), msg.size());
    bin = std::move(msgBin);
    ++m_msg_handled;
    return true;
  }
  return false;
}
bool ZMQPairReceiver::sleep_receive(daqling::utilities::Binary &bin) {
  zmq::message_t msg;
  if (m_socket->recv(&msg)) {
    utilities::Binary msgBin(msg.data(), msg.size());
    bin = std::move(msgBin);
    ++m_msg_handled;
    return true;
  }
  return false;
}