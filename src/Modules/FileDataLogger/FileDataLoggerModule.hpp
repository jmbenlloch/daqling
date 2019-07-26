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

  /*
   * A wrapper around a printf-like output file generator.
   */
  class FileGenerator {
  public:
    FileGenerator(const std::string pattern) : m_pattern(pattern) {}
    std::ofstream next();

  private:
    const std::string m_pattern;
    unsigned m_filenum = 0;
  };

  void pager();
  void flusher();

  // Configs
  /* long m_pagesize; */
  long m_max_filesize;

  // Internals
  folly::ProducerConsumerQueue<daqling::utilities::Binary> m_payloads;
  /* daqling::utilities::Binary m_buffer; */
  std::map<uint64_t, std::unique_ptr<daqling::utilities::ReusableThread>> m_fileWriters;
  /* std::map<uint64_t, std::function<void()>> m_writeFunctors; */
  /* std::map<uint64_t, std::string> m_fileNames; */
  std::map<uint64_t, std::ofstream> m_fileStreams;
  /* std::map<uint64_t, std::tuple<daqling::utilities::Binary, std::unique_ptr<std::mutex>, std::unique_ptr<std::condition_variable>>> m_fileBuffers; */
  /* std::map<uint64_t, uint32_t> m_fileRotationCounters; */
  long m_filenum = 0;

  std::atomic<int> m_bytes_sent;
  std::unique_ptr<std::thread> m_monitor_thread;
  // Thread control
  std::atomic<bool> m_stopWriters;

  FileGenerator m_fileGenerator;
};
