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
#include "Singleton.hpp"
// #include <boost/thread/shared_mutex.hpp>
#include <map>
#include <shared_mutex>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>
#include <utility>
namespace daqling {
namespace utilities {

class ThreadTagger : public Singleton<ThreadTagger> {
private:
  /* data */
  std::map<pid_t, std::string> m_tags_map;
  std::shared_mutex _reader_writer_lock;

public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
  std::string readTagCurrentThread() { return readTag(syscall(SYS_gettid)); }
  std::string readTag(pid_t pid) {
    std::shared_lock<std::shared_mutex> lock(_reader_writer_lock);
    if (m_tags_map.find(pid) != m_tags_map.end()) {
      return m_tags_map[pid];
    }
    // potentially percolate up through thread tree?
    return "unknown";
  }
  void writeTag(std::string tag) {
    // get pid
    std::unique_lock<std::shared_mutex> lock(_reader_writer_lock);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    m_tags_map[static_cast<pid_t>(syscall(SYS_gettid))] = std::move(tag);
  }
};

} // namespace utilities
} // namespace daqling