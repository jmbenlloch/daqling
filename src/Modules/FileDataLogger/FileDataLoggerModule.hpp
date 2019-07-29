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

#pragma once

/// \cond
#include <fstream>
#include <map>
#include <queue>
#include <memory>
#include <tuple>
#include <condition_variable>
/// \endcond

#include "Core/DAQProcess.hpp"
#include "Core/DataLogger.hpp"
#include "Utils/Binary.hpp"
#include "Utils/ProducerConsumerQueue.hpp"

/*
 * FileDataLoggerModule
 * Description: Data logger for binary files with fstream.
 *   Relies on fixed size file IO with Binary splitting and concatenation.
 * Date: April 2019
 */
class FileDataLoggerModule : public daqling::core::DAQProcess, public daqling::core::DataLogger {
 public:
  FileDataLoggerModule();
  ~FileDataLoggerModule();

  void start();
  void stop();
  void runner();

  void monitor_runner();

  void setup();
  void write();
  void read();
  bool write(uint64_t keyId, daqling::utilities::Binary& payload);
  void shutdown();

 private:
  struct ThreadContext {
    ThreadContext(std::array<int, 2> tids) : consumer(tids[0]), producer(tids[1]) {}
    daqling::utilities::ReusableThread consumer;
    daqling::utilities::ReusableThread producer;
  };
  using PayloadQueue = folly::ProducerConsumerQueue<daqling::utilities::Binary>;
  using Context = std::tuple<PayloadQueue, ThreadContext>;

  /*
   * A wrapper around a printf-like output file generator.
   */
  class FileGenerator {
  public:
    FileGenerator(const std::string pattern, const uint64_t chid) : m_pattern(pattern), m_chid(chid) {}
    std::ofstream next();

  private:
    const std::string m_pattern;
    const uint64_t m_chid;
    unsigned m_filenum = 0;
  };

  // Configs
  long m_max_filesize;
  uint64_t m_channels = 0;

  // Thread control
  std::atomic<bool> m_stopWriters;

  // Internals
  void flusher(const uint64_t chid, PayloadQueue &pq, const long max_buffer_size, FileGenerator &&fg) const;
  std::map<uint64_t, Context> m_channelContexts;
  mutable std::atomic<int> m_bytes_sent;
  std::thread m_monitor_thread;

};
