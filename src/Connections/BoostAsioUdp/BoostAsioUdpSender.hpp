#pragma once
#include "Core/Sender.hpp"
#include "nlohmann/json.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <memory>

namespace daqling {
namespace connection {
using namespace boost::asio::ip;

class BoostAsioUdpSender : public daqling::core::Sender {
public:
  BoostAsioUdpSender(uint chid, const nlohmann::json &j = NULL);

protected:
  bool send(const daqling::utilities::Binary &bin) override;
  bool sleep_send(const daqling::utilities::Binary &bin) override;

  boost::asio::io_context m_io_context;
  std::unique_ptr<udp::endpoint> m_dest_endpoint;
  std::unique_ptr<udp::socket> m_socket;
  std::unique_ptr<boost::asio::deadline_timer> m_timeout;
};
} // namespace connection
} // namespace daqling