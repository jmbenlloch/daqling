#include "Queue.hpp"
#include <chrono>
#include <ctime>
#include <thread>

using namespace daqling::core;
using namespace std::chrono_literals;

bool Queue::sleep_read(daqling::utilities::Binary &bin) {
  if (read(bin)) {
    return true;
  }
  std::this_thread::sleep_for((std::chrono::milliseconds(m_sleep_duration)));
  return false;
}
bool Queue::sleep_write(const daqling::utilities::Binary &bin) {
  if (write(bin)) {
    return true;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(m_sleep_duration));
  return false;
}
void Queue::set_sleep_duration(uint ms) { m_sleep_duration = ms; }