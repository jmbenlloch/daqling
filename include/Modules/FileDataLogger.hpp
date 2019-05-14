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

#ifndef DAQ_MODULES_FILEDATALOGGER_HPP_
#define DAQ_MODULES_FILEDATALOGGER_HPP_

/// \cond
#include <iostream>
#include <fstream>
#include <queue>
#include <map>
/// \endcond

#include "Core/DAQProcess.hpp"
#include "Core/DataLogger.hpp"
#include "Utilities/Binary.hpp"
#include "Utilities/ChunkedStorage.hpp"
#include "Utilities/ProducerConsumerQueue.hpp"


/*
 * FileDataLogger
 * Author: Roland.Sipos@cern.ch
 * Description: Data logger for binary files with fstream.
 *   Relies on fixed size file IO with Binary splitting and concatenation.
 * Date: April 2019
 */
class FileDataLogger : public DAQProcess, public DataLogger {
public:
  FileDataLogger();
  ~FileDataLogger();

  void start();
  void stop();
  void runner();

  void setup();
  void write();
  void read();
  bool write(uint64_t keyId, daq::utilities::Binary& payload);
  void shutdown();

private:
  // Configs
  long m_writeBytes;

  // Internals 
  folly::ProducerConsumerQueue<daq::utilities::Binary> m_payloads;
  daq::utilities::Binary m_buffer;
  std::map<uint64_t, std::unique_ptr<daq::utilities::ReusableThread>> m_fileWriters;
  std::map<uint64_t, std::function<void()>> m_writeFunctors;
  std::map<uint64_t, std::string> m_fileNames; 
  std::map<uint64_t, std::fstream> m_fileStreams;
  std::map<uint64_t, daq::utilities::Binary> m_fileBuffers;
  std::map<uint64_t, uint32_t> m_fileRotationCounters; 

  // Thread control
  std::atomic<bool> m_stopWriters;

};

#endif /* DAQ_MODULES_FILEDATALOGGER_HPP_ */
