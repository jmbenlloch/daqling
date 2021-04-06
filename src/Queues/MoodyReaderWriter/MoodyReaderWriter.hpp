#pragma once

#include "Core/Queue.hpp"
// #include "Utils/atomicops.hpp"
#include "moody/readerwriterqueue.h"
#include "nlohmann/json.hpp"

using MessageQueue = moodycamel::BlockingReaderWriterQueue<daqling::utilities::Binary>;
namespace daqling {
namespace queue {

class MoodyReaderWriter : public daqling::core::Queue {
public:
  MoodyReaderWriter(const nlohmann::json &j);
  bool read(daqling::utilities::Binary & /*bin*/) override;
  // bool sleep_read(daqling::utilities::Binary&) override;
  bool write(const daqling::utilities::Binary & /*bin*/) override;
  uint sizeGuess() override;
  uint capacity() override;

protected:
private:
  unsigned int m_capacity;
  MessageQueue m_queue;
};
} // namespace queue
} // namespace daqling