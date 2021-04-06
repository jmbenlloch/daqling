#pragma once

#include "Core/Queue.hpp"
#include "folly/ProducerConsumerQueue.h"
#include "nlohmann/json.hpp"

using MessageQueue = folly::ProducerConsumerQueue<daqling::utilities::Binary>;
namespace daqling {
namespace queue {

class FollyProducerConsumer : public daqling::core::Queue {
public:
  FollyProducerConsumer(const nlohmann::json &j);
  bool read(daqling::utilities::Binary & /*bin*/) override;
  bool write(const daqling::utilities::Binary & /*bin*/) override;
  uint sizeGuess() override;
  uint capacity() override;

protected:
private:
  MessageQueue m_queue;
};
} // namespace queue
} // namespace daqling