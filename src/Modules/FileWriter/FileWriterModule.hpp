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
#include <condition_variable>
#include <fstream>
#include <map>
#include <memory>
#include <queue>
#include <tuple>
/// \endcond

#include "Core/DAQProcess.hpp"
#include "Core/DataLogger.hpp"
#include "Utils/Binary.hpp"
#include "Utils/ProducerConsumerQueue.hpp"

/**
 * Module for writing your acquired data to file.
 */
class FileWriterModule : public daqling::core::DAQProcess, public daqling::core::DataLogger {
public:
  FileWriterModule();
  ~FileWriterModule();

  void start(unsigned run_num);
  void stop();
  void runner();

  void monitor_runner();

  void setup();
  void write();
  void read();
  bool write(uint64_t keyId, daqling::utilities::Binary &payload);
  void shutdown();

private:
  struct ThreadContext {
    ThreadContext(std::array<unsigned int, 2> tids) : consumer(tids[0]), producer(tids[1]) {}
    daqling::utilities::ReusableThread consumer;
    daqling::utilities::ReusableThread producer;
  };
  using PayloadQueue = folly::ProducerConsumerQueue<daqling::utilities::Binary>;
  using Context = std::tuple<PayloadQueue, ThreadContext>;

  struct Metrics {
    std::atomic<size_t> bytes_written = 0;
    std::atomic<size_t> payload_queue_size = 0;
    std::atomic<size_t> payload_size = 0;
  };

  size_t m_buffer_size;
  std::string m_pattern;
  std::atomic<bool> m_start_completed;

  /**
   * Output file generator with a printf-like filename pattern.
   */
  class FileGenerator {
  public:
    FileGenerator(const std::string pattern, const uint64_t chid, const unsigned run_number)
        : m_pattern(pattern), m_chid(chid), m_run_number(run_number) {}

    /**
     * Generates the next output file in the sequence.
     *
     * @warning Silently overwrites files if they already exists
     * @warning Silently overwrites previous output files if specified pattern does not generate
     * unique file names.
     */
    std::ofstream next();

    /**
     * Returns whether `pattern` yields unique output files on rotation.
     * Effectively checks whether the pattern contains %n.
     */
    static bool yields_unique(const std::string &pattern);

  private:
    const std::string m_pattern;
    const uint64_t m_chid;
    unsigned m_filenum = 0;
    const unsigned m_run_number;
  };

  // Configs
  size_t m_max_filesize;
  uint64_t m_channels = 0;

  // Thread control
  std::atomic<bool> m_stopWriters;

  // Metrics
  mutable std::map<uint64_t, Metrics> m_channelMetrics;

  // Internals
  void flusher(const uint64_t chid, PayloadQueue &pq, const size_t max_buffer_size,
               FileGenerator fg) const;
  std::map<uint64_t, Context> m_channelContexts;
  std::thread m_monitor_thread;
};
