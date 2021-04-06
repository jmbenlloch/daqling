#pragma once
#include "Core/Sender.hpp"
#include "nlohmann/json.hpp"
#include <memory>
#include <zmq.hpp>

namespace daqling {
namespace connection {

class ZMQPubSubSender : public daqling::core::Sender {
public:
  ZMQPubSubSender(uint chid, const nlohmann::json &j);

protected:
  bool send(const daqling::utilities::Binary &bin) override;

  // ZMQ Context number of threads to use
  uint8_t ioT = 1;
  std::unique_ptr<zmq::context_t> m_context;
  std::unique_ptr<zmq::socket_t> m_socket;
};
} // namespace connection
} // namespace daqling