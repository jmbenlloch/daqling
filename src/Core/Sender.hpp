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
class Sender {
public:
  // virtual destructor
  virtual ~Sender() = default;

  virtual bool send(const daqling::utilities::Binary &bin) = 0;
  virtual bool sleep_send(const daqling::utilities::Binary &bin);
  virtual bool start();
  virtual bool stop();
  virtual void set_sleep_duration(uint ms);
  virtual std::atomic<size_t> &getMsgsHandled();
  virtual std::atomic<size_t> &getPcqSize();

protected:
  Sender(uint chid);
  uint m_chid;
  std::atomic<size_t> m_msg_handled{};
  std::atomic<size_t> m_pcq_size{};
  uint m_sleep_duration{};

private:
};

class QueueSender : public Sender {
public:
  QueueSender(uint /*chid*/, std::unique_ptr<Queue> /*ptr*/);
  bool send(const daqling::utilities::Binary &bin) override;
  bool sleep_send(const daqling::utilities::Binary &bin) override;
  bool start() override;
  bool stop() override;
  virtual void setChainedSender(std::shared_ptr<daqling::core::Sender> /*ptr*/);
  std::atomic<size_t> &getMsgsHandled() override;
  void set_sleep_duration(uint ms) override;

protected:
  bool addSendHandler();

  std::thread m_handler_thread;
  std::shared_ptr<daqling::core::Sender> m_chained_sender;
  std::atomic<bool> m_stop_handler{};
  std::unique_ptr<Queue> m_queue;
};
} // namespace core
} // namespace daqling
