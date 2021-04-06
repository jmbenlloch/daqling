#include "DummySender.hpp"
#include "Utils/ConnectionMacros.hpp"
#include "Utils/Ers.hpp"

using namespace daqling::connection;

REGISTER_SENDER(DummySender, "Dummy")

DummySender::DummySender(uint chid, const nlohmann::json &j) : daqling::core::Sender(chid) {
  if (j.empty()) {
  }
}

bool DummySender::send(const daqling::utilities::Binary &bin) {
  ERS_DEBUG(0, "Hello from DummySender protocolHandler");
  const auto &bino(bin);
  return bino == bin;
}