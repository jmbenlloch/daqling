#pragma once
#include "Core/Receiver.hpp"
#include "nlohmann/json.hpp"
#include <memory>
#include <zmq.hpp>

namespace daqling {
namespace connection {
class ZMQPairReceiver : public daqling::core::Receiver {
public:
  ZMQPairReceiver(uint chid, const nlohmann::json &j);
  void set_sleep_duration(uint ms) override;

protected:
  bool receive(daqling::utilities::Binary &bin) override;
  bool sleep_receive(daqling::utilities::Binary &bin) override;
  // ZMQ Context number of threads to use
  uint8_t ioT = 1;
  std::unique_ptr<zmq::context_t> m_context;
  std::unique_ptr<zmq::socket_t> m_socket;
};
} // namespace connection
} // namespace daqling