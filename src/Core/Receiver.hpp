#pragma once
#include "Queue.hpp"
#include "Utils/Binary.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
namespace daqling {
namespace core {
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Receiver {
public:
  // Virtual destructor
  virtual ~Receiver() = default;
  virtual bool receive(daqling::utilities::Binary &bin) = 0;
  virtual bool sleep_receive(daqling::utilities::Binary &bin);
  virtual void set_sleep_duration(uint ms);
  virtual bool start();
  virtual bool stop();
  virtual std::atomic<size_t> &getMsgsHandled();
  virtual std::atomic<size_t> &getPcqSize();

protected:
  Receiver(uint chid);
  uint m_chid;
  std::atomic<size_t> m_msg_handled{};
  std::atomic<size_t> m_pcq_size{};
  uint m_sleep_duration{};
};

class QueueReceiver : public Receiver {
public:
  QueueReceiver(uint /*chid*/, std::unique_ptr<Queue> /*ptr*/);
  bool receive(daqling::utilities::Binary &bin) override;
  bool sleep_receive(daqling::utilities::Binary &bin) override;
  bool start() override;
  bool stop() override;
  virtual void setChainedReceiver(std::shared_ptr<daqling::core::Receiver> /*ptr*/);
  std::atomic<size_t> &getMsgsHandled() override;
  void set_sleep_duration(uint ms) override;

protected:
  bool addReceiveHandler();

  std::thread m_handler_thread;
  std::shared_ptr<daqling::core::Receiver> m_chained_receiver;
  std::atomic<bool> m_stop_handler{};
  std::unique_ptr<Queue> m_queue;
};
} // namespace core
} // namespace daqling
