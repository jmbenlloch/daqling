#include "MoodyReaderWriter.hpp"
#include "Utils/ConnectionMacros.hpp"

using namespace daqling::queue;
REGISTER_QUEUE(MoodyReaderWriter, "ReaderWriter")
MoodyReaderWriter::MoodyReaderWriter(const nlohmann::json &j)
    : m_queue(j.at("queue_size").get<unsigned int>()) {
  m_capacity = j.at("queue_size").get<unsigned int>();
}

bool MoodyReaderWriter::read(daqling::utilities::Binary &bin) { return m_queue.try_dequeue(bin); }
// bool MoodyReaderWriter::sleep_read(daqling::utilities::Binary& bin)
// {
//     return m_queue.wait_dequeue_timed(bin,std::chrono::milliseconds(m_sleep_duration));
// }
bool MoodyReaderWriter::write(const daqling::utilities::Binary &bin) {
  // return m_queue.try_emplace(std::move(bin));
  return m_queue.try_enqueue(bin);
}
uint MoodyReaderWriter::sizeGuess() { return m_queue.size_approx(); }
uint MoodyReaderWriter::capacity() { return m_capacity; }