/**
 * Copyright (C) 2019-2021 CERN
 *
 * DAQling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * DAQling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with DAQling. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "Common/DataType.hpp"
#include "Queue.hpp"
#include <atomic>
#include <mutex>
#include <thread>
namespace daqling {
namespace core {
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Sender {
public:
  // virtual destructor
  virtual ~Sender() = default;
  virtual bool send(DataType &bin) = 0;
  virtual bool sleep_send(DataType &bin);
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

template <class T> class QueueSender : public Sender {
public:
  QueueSender(uint /*chid*/, std::unique_ptr<Queue> /*ptr*/,
              const std::shared_ptr<Sender> & /*chained_ptr*/);
  bool send(DataType &bin) override;
  bool sleep_send(DataType &bin) override;
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

#include <chrono>
#include <ctime>
#include <utility>
using namespace std::chrono_literals;
// Queue sender
template <class T>
QueueSender<T>::QueueSender(uint chid, std::unique_ptr<Queue> ptr,
                            const std::shared_ptr<Sender> &chained_ptr)
    : Sender(chid) {
  m_queue = std::move(ptr);
  setChainedSender(chained_ptr);
}
template <class T> bool QueueSender<T>::send(DataType &bin) { return m_queue->write(bin); }
template <class T> bool QueueSender<T>::sleep_send(DataType &bin) {
  return m_queue->sleep_write(bin);
}
template <class T> void QueueSender<T>::set_sleep_duration(uint ms) {
  m_sleep_duration = ms;
  if (m_queue) {
    m_queue->set_sleep_duration(ms);
  }
}
template <class T> bool QueueSender<T>::start() {
  m_stop_handler.store(false);
  return (addSendHandler() && m_chained_sender->start());
}
template <class T> bool QueueSender<T>::stop() {
  m_stop_handler.store(true);
  bool retval = m_chained_sender->stop();
  m_handler_thread.join();
  return retval;
}
template <class T> bool QueueSender<T>::addSendHandler() {
  ERS_INFO(" [SERVER] SendHandler for channel [" << m_chid << "] starting...");
  m_handler_thread = std::thread([&]() {
    while (!m_stop_handler) {
      if (m_queue->sizeGuess() != 0) {
        T m_container;
        if (m_queue->sleep_read(m_container)) {
          m_chained_sender->send(m_container);
        }
      } else {
        std::this_thread::sleep_for(1ms);
      }
      m_pcq_size.store(m_queue->sizeGuess());
      if (m_queue->sizeGuess() > m_queue->capacity() * 0.9) {
        ERS_WARNING("SERVER -> queue population: " << m_queue->sizeGuess());
      }
    }
    ERS_INFO(" joining channel [" << m_chid << "] handler.");
  });
  return true;
}
template <class T>
void QueueSender<T>::setChainedSender(std::shared_ptr<daqling::core::Sender> ptr) {
  m_chained_sender = std::move(ptr);
  m_chained_sender->set_sleep_duration(1);
}
template <class T> std::atomic<size_t> &QueueSender<T>::getMsgsHandled() {
  return m_chained_sender->getMsgsHandled();
}
} // namespace core
} // namespace daqling
