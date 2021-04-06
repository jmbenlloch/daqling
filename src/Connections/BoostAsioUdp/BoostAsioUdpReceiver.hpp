#pragma once
#include "Core/Receiver.hpp"
#include "nlohmann/json.hpp"
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <memory>

namespace daqling {
namespace connection {
using namespace boost::asio::ip;

class BoostAsioUdpReceiver : public daqling::core::Receiver {
public:
  BoostAsioUdpReceiver(uint chid, const nlohmann::json &j = NULL);
  bool stop() override;

protected:
  bool receive(daqling::utilities::Binary &bin) override;
  bool sleep_receive(daqling::utilities::Binary &bin) override;
  void handle_receive(const boost::system::error_code & /*error*/, std::size_t /*size*/);
  void wait_callback(const boost::system::error_code &error);

  boost::asio::io_context m_io_context;
  std::unique_ptr<udp::endpoint> m_dest_endpoint;
  std::unique_ptr<udp::endpoint> m_src_endpoint;
  std::unique_ptr<udp::socket> m_socket;
  std::unique_ptr<boost::asio::deadline_timer> m_timeout;
  boost::array<char, 2048> m_recv_buf{};
  size_t m_len{};
};
} // namespace connection
} // namespace daqling