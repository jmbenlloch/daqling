/**
 * Copyright (C) 2019 CERN
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

#ifndef DAQLING_UTILITIES_REUSABLETHREAD_HPP
#define DAQLING_UTILITIES_REUSABLETHREAD_HPP

/// \cond
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
/// \endcond

/********************************
 * ReusableThread
 * Copied from: 
 *   // https://codereview.stackexchange.com/questions/134214/reuseable-c11-thread
 * Description: Pausable thread
 * Date: November 2017
 *********************************/

namespace daqling {
namespace utilities {


class ReusableThread
{
public:
  ReusableThread(unsigned int threadID)
    : m_thread_id(threadID), m_thread_pause(true), m_thread_quit(false), m_function_ready(false),
      m_thread(&ReusableThread::thread_worker, this)
  { }

  ~ReusableThread()
  {
    m_thread_quit = true;
    m_cv.notify_all();
    m_thread.join();
  }

  std::thread& get_thread() { return std::ref(m_thread); }

  unsigned int get_thread_id() const { return m_thread_id; }

  bool get_readiness() const { return m_thread_pause; }

  bool set_work(const std::function<void()>& work_func)
  {
    if (!m_function_ready && m_thread_pause.exchange(false)) {
      m_work_func = work_func;
      m_function_ready = true;
      m_cv.notify_all();
      return true;
    }
    return false;
  }

private:
  unsigned int m_thread_id;
  std::atomic<bool> m_thread_pause;
  std::atomic<bool> m_thread_quit;
  std::atomic<bool> m_function_ready;
  std::function<void()> m_work_func;

  std::mutex m_mtx;
  std::condition_variable m_cv;
  std::thread m_thread;

  void thread_worker()
  {
    std::unique_lock<std::mutex> lock(m_mtx); // <- lock the mutex
    while (!m_thread_quit){
      if (!m_thread_pause && m_function_ready){
        m_work_func();
        m_thread_pause = true;
        m_function_ready = false;
      } else {
        m_cv.wait(lock);
      }
    }
  }

};

} // namespace utilities
} // namespace daqling

#endif // DAQLING_UTILITIES_REUSABLETHREAD_HPP

