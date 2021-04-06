#include "BoostAsioUdpReceiver.hpp"
#include "BoostAsioUdpIssues.hpp"
#include "Utils/ConnectionMacros.hpp"
#include "Utils/Ers.hpp"
#include <boost/bind.hpp>

using namespace daqling::connection;

REGISTER_RECEIVER(BoostAsioUdpReceiver, "BoostAsioUdp")
BoostAsioUdpReceiver::BoostAsioUdpReceiver(uint chid, const nlohmann::json &j)
    : daqling::core::Receiver(chid), m_io_context() {
  std::string connStr;
  if (j.at("transport") == "udp") {
    try {
      int src_port = (j.contains("src_port") ? j.at("src_port").get<int>() : 0);
      auto host = boost::asio::ip::address::from_string(j.at("host").get<std::string>());
      m_socket = std::make_unique<udp::socket>(m_io_context);
      m_dest_endpoint = std::make_unique<udp::endpoint>(host, j.at("dest_port").get<int>());
      m_src_endpoint =
          std::make_unique<udp::endpoint>(boost::asio::ip::address_v4::any(), src_port);
      m_socket->open(udp::v4());
      m_socket->bind(*m_dest_endpoint);
      m_timeout = std::make_unique<boost::asio::deadline_timer>(m_io_context);
    } catch (const std::exception &e) {
      throw CannotAddChannel(ERS_HERE, e.what());
    }
  } else {
    throw InvalidTransportType(ERS_HERE, j.at("transport").get<std::string>().c_str());
  }
}
bool BoostAsioUdpReceiver::receive(daqling::utilities::Binary &bin) {
  m_socket->async_receive_from(boost::asio::buffer(m_recv_buf), *m_src_endpoint,
                               boost::bind(&BoostAsioUdpReceiver::handle_receive, this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
  m_timeout->expires_from_now(boost::posix_time::milliseconds(1));
  m_timeout->async_wait(
      boost::bind(&BoostAsioUdpReceiver::wait_callback, this, boost::asio::placeholders::error));
  m_io_context.restart();
  m_io_context.run();
  if (m_len != 0u) {
    ERS_DEBUG(0, "Received msg with size: " << m_len);
    utilities::Binary msgBin(m_recv_buf.data(), m_len);
    bin = std::move(msgBin);
    ++m_msg_handled;
    return true;
  }
  return false;
}
bool BoostAsioUdpReceiver::sleep_receive(daqling::utilities::Binary &bin) {
  m_socket->async_receive_from(boost::asio::buffer(m_recv_buf), *m_src_endpoint,
                               boost::bind(&BoostAsioUdpReceiver::handle_receive, this,
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred));
  m_timeout->expires_from_now(boost::posix_time::milliseconds(m_sleep_duration));
  m_timeout->async_wait(
      boost::bind(&BoostAsioUdpReceiver::wait_callback, this, boost::asio::placeholders::error));
  m_io_context.restart();
  m_io_context.run();
  if (m_len != 0u) {
    ERS_DEBUG(0, "Received msg with size: " << m_len);
    utilities::Binary msgBin(m_recv_buf.data(), m_len);
    bin = std::move(msgBin);
    ++m_msg_handled;
    return true;
  }
  return false;
}
bool BoostAsioUdpReceiver::stop() {
  m_socket->cancel();
  return true;
}
void BoostAsioUdpReceiver::wait_callback(const boost::system::error_code &error) {
  if (error) {
    return;
  }
  m_socket->cancel();
}
void BoostAsioUdpReceiver::handle_receive(const boost::system::error_code &error,
                                          std::size_t size) {
  if (error || (size == 0u)) {
    m_len = 0;
  } else {
    m_timeout->cancel();
    m_len = size;
  }
}