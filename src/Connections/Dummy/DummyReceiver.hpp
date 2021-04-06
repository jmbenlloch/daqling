#pragma once
#include "Core/Receiver.hpp"
#include "nlohmann/json.hpp"

namespace daqling {
namespace connection {
class DummyReceiver : public daqling::core::Receiver {
public:
  DummyReceiver(uint chid, const nlohmann::json &j = NULL);

protected:
  bool receive(daqling::utilities::Binary &bin) override;
};
} // namespace connection
} // namespace daqling