#include "FollyProducerConsumer.hpp"
#include "Utils/ConnectionMacros.hpp"

using namespace daqling::queue;
REGISTER_QUEUE(FollyProducerConsumer, "FollyProducerConsumer")
FollyProducerConsumer::FollyProducerConsumer(const nlohmann::json &j)
    : m_queue(j.at("queue_size").get<unsigned int>()) {}

bool FollyProducerConsumer::read(daqling::utilities::Binary &bin) { return m_queue.read(bin); }
bool FollyProducerConsumer::write(const daqling::utilities::Binary &bin) {
  return m_queue.write(bin);
}
uint FollyProducerConsumer::sizeGuess() { return m_queue.sizeGuess(); }
uint FollyProducerConsumer::capacity() { return m_queue.capacity(); }