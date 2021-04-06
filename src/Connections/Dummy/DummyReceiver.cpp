#include "DummyReceiver.hpp"
#include "Utils/ConnectionMacros.hpp"
#include "Utils/Ers.hpp"

using namespace daqling::connection;

REGISTER_RECEIVER(DummyReceiver, "Dummy")
DummyReceiver::DummyReceiver(uint chid, const nlohmann::json &j) : daqling::core::Receiver(chid) {
  if (j.empty()) {
  }
}
bool DummyReceiver::receive(daqling::utilities::Binary &bin) {
  const auto &bino(bin);
  ERS_DEBUG(0, "Hello from DummyReceiver protocolHandler");
  return bino == bin;
}