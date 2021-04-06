#include "BoostAsioUdpSender.hpp"
#include "BoostAsioUdpIssues.hpp"
#include "Utils/ConnectionMacros.hpp"
#include "Utils/Ers.hpp"
#include <boost/bind.hpp>

using namespace daqling::connection;

REGISTER_SENDER(BoostAsioUdpSender, "BoostAsioUdp")

BoostAsioUdpSender::BoostAsioUdpSender(uint chid, const nlohmann::json &j)
    : daqling::core::Sender(chid) {
  if (j.at("transport") == "udp") {
    try {
      int src_port = (j.contains("src_port") ? j.at("src_port").get<int>() : 0);
      auto host = boost::asio::ip::address::from_string(j.at("host").get<std::string>());
      m_socket = std::make_unique<udp::socket>(m_io_context, udp::endpoint(host, src_port));
      m_dest_endpoint = std::make_unique<udp::endpoint>(host, j.at("dest_port").get<int>());
      m_timeout = std::make_unique<boost::asio::deadline_timer>(m_io_context);
    } catch (const std::exception &e) {
      throw CannotAddChannel(ERS_HERE, e.what());
    }
  } else {
    throw InvalidTransportType(ERS_HERE, j.at("transport").get<std::string>().c_str());
  }
}

bool BoostAsioUdpSender::send(const daqling::utilities::Binary &bin) {
  try {
    ERS_DEBUG(0, "Sending message of size: " << bin.size());

    if (m_socket->send_to(boost::asio::buffer(bin.data(), bin.size()), *m_dest_endpoint) ==
        bin.size()) {
      ++m_msg_handled;
      return true;
    }
    return false;

  } catch (std::exception &e) {
    throw SendFailed(ERS_HERE, e.what());
  }
}
bool BoostAsioUdpSender::sleep_send(const daqling::utilities::Binary &bin) {
  try {
    uint len;
    m_socket->async_send_to(boost::asio::buffer(bin.data(), bin.size()), *m_dest_endpoint,
                            [&](const boost::system::error_code &error, std::size_t size) {
                              if (error || (size == 0u)) {
                                len = 0;
                              } else {
                                m_timeout->cancel();
                                len = size;
                              }
                            });
    ERS_DEBUG(0, "Sending message of size: " << bin.size());
    m_timeout->expires_from_now(boost::posix_time::milliseconds(m_sleep_duration));
    m_timeout->async_wait([&](const boost::system::error_code &error) {
      if (error) {
        return;
      }
      m_socket->cancel();
    });
    m_io_context.restart();
    m_io_context.run();
    if (len == bin.size()) {
      ++m_msg_handled;
      return true;
    }
    ERS_DEBUG(0, "Sleep send failed.");
    return false;

  } catch (std::exception &e) {
    throw SendFailed(ERS_HERE, e.what());
  }
}