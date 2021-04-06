#pragma once
#include "Core/Sender.hpp"
#include "nlohmann/json.hpp"

namespace daqling {
namespace connection {

class DummySender : public daqling::core::Sender {
public:
  DummySender(uint chid, const nlohmann::json &j = NULL);

protected:
  bool send(const daqling::utilities::Binary &bin) override;
};
} // namespace connection
} // namespace daqling