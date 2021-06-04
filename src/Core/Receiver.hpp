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
#include <thread>
namespace daqling {
namespace core {
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Receiver {
public:
  // Virtual destructor
  virtual ~Receiver() = default;

  virtual bool receive(DataType &bin) = 0;
  virtual bool sleep_receive(DataType &bin);
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
template <class T> class QueueReceiver : public Receiver {
public:
  QueueReceiver(uint /*chid*/, std::unique_ptr<Queue> /*ptr*/,
                const std::shared_ptr<Receiver> & /*chained_ptr*/);
  bool receive(DataType &bin) override;
  bool sleep_receive(DataType &bin) override;
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

#include <chrono>
#include <ctime>
#include <utility>
using namespace std::chrono_literals;
// Queue receiver
template <class T>
QueueReceiver<T>::QueueReceiver(uint chid, std::unique_ptr<Queue> ptr,
                                const std::shared_ptr<Receiver> &chained_ptr)
    : Receiver(chid) {
  m_queue = std::move(ptr);
  setChainedReceiver(chained_ptr);
}
template <class T> bool QueueReceiver<T>::receive(DataType &bin) {
  if (m_queue->sizeGuess() != 0) {

    return m_queue->read(bin);
  }
  return false;
}
template <class T> void QueueReceiver<T>::set_sleep_duration(uint ms) {
  m_sleep_duration = ms;
  if (m_queue) {
    m_queue->set_sleep_duration(ms);
  }
}
template <class T> bool QueueReceiver<T>::sleep_receive(DataType &bin) {
  if (m_queue->sizeGuess() != 0) {

    return m_queue->sleep_read(bin);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(m_sleep_duration));
  return false;
}
template <class T> bool QueueReceiver<T>::start() {
  m_stop_handler.store(false);
  return (addReceiveHandler() && m_chained_receiver->start());
}
template <class T> bool QueueReceiver<T>::stop() {
  m_stop_handler.store(true);
  bool retval = m_chained_receiver->stop();
  m_handler_thread.join();
  return retval;
}
template <class T> bool QueueReceiver<T>::addReceiveHandler() {
  ERS_INFO(" [CLIENT] ReceiveHandler for channel [" << m_chid << "] starting...");
  m_handler_thread = std::thread([&]() {
    while (!m_stop_handler) {
      T m_container;
      if (m_chained_receiver->sleep_receive(m_container)) {
        while (!m_queue->sleep_write(m_container) && !m_stop_handler) {
          ERS_WARNING("Waiting queue to allow write");
        }
      }
      m_pcq_size.store(m_queue->sizeGuess());
      if (m_queue->sizeGuess() > m_queue->capacity() * 0.9) {
        ERS_WARNING("CLIENT -> queue population: " << m_queue->sizeGuess());
      }
    }
    ERS_INFO(" joining channel [" << m_chid << "] handler.");
  });
  return true;
}
template <class T>
void QueueReceiver<T>::setChainedReceiver(std::shared_ptr<daqling::core::Receiver> ptr) {
  m_chained_receiver = std::move(ptr);
  m_chained_receiver->set_sleep_duration(1);
}
template <class T> std::atomic<size_t> &QueueReceiver<T>::getMsgsHandled() {
  return m_chained_receiver->getMsgsHandled();
}
} // namespace core
} // namespace daqling
