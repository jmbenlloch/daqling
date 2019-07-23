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

/// \cond
#include <chrono>
#include <sstream>
/// \endcond

#include "FileDataLoggerModule.hpp"
#include "Utils/Logging.hpp"


using namespace std::chrono_literals;
namespace daqutils = daqling::utilities;

FileDataLoggerModule::FileDataLoggerModule() : m_payloads{10000}, m_bytes_sent{0}, m_stopWriters{false} {
  INFO(__METHOD_NAME__);

/* #warning RS -> Needs to be properly configured. */
  // Set up static resources...
  std::ios_base::sync_with_stdio(false);
  m_fileNames[1] = "/tmp/test.bin";
  m_fileStreams[1] = std::fstream(m_fileNames[1], std::ios::out | std::ios::binary);
  m_fileBuffers[1] = daqling::utilities::Binary(0);
  setup();
}

FileDataLoggerModule::~FileDataLoggerModule() {
  INFO(__METHOD_NAME__);
  // Tear down resources...
  m_stopWriters.store(true);
  m_fileStreams[1].close();
}

void FileDataLoggerModule::start() {
  DAQProcess::start();
  INFO(" getState: " << getState());
  m_monitor_thread = std::make_unique<std::thread>(&FileDataLoggerModule::monitor_runner, this);
}

void FileDataLoggerModule::stop() {
  DAQProcess::stop();
  INFO(" getState: " << this->getState());
  m_monitor_thread->join();
  INFO("Joined successfully monitor thread");
}

void FileDataLoggerModule::runner() {
  INFO(" Running...");
  // auto& cm = daqling::core::ConnectionManager::instance();
  while (m_run) {
    daqutils::Binary pl(0);
    while (!m_connections.get(1, std::ref(pl)) && m_run) {
      std::this_thread::sleep_for(1ms);
    }
    m_payloads.write(pl);
    // DEBUG("Wrote data from channel 1...");
  }
  INFO(" Runner stopped");
}

#warning RS -> File rotation implementation is missing
#warning RS -> Hardcoded values should come from config.
void FileDataLogger::setup() {
  const long pagesize = std::invoke([this]() -> long {
    try {
      return std::stoi(std::string(m_config.getConfig()["settings"]["pagesize"]));
    } catch (const nlohmann::json::exception&) {
      return 4 * daqutils::Constant::Kilo; // 4K buffer
    }
  });

  // Loop through sources from config and add a file writer for each sink.
  unsigned tid = 1;
  m_fileWriters[tid] = std::make_unique<daqling::utilities::ReusableThread>(11111);
  m_writeFunctors[tid] = [&, tid, pagesize] {
    int ftid = tid;
    INFO(" Spawning fileWriter for link: " << ftid);
    while (!m_stopWriters) {
      if (m_payloads.sizeGuess() > 0) {
        DEBUG(" SIZES: Queue pop.: " << m_payloads.sizeGuess()
                              << " loc.buff. size: " << m_fileBuffers[ftid].size()
                              << " payload size: " << m_payloads.frontPtr()->size());
        const long sizeSum = static_cast<long>(m_fileBuffers[ftid].size()) + m_payloads.frontPtr()->size();
        if (sizeSum > pagesize) {  // Split needed.

          DEBUG(" Processing split.");

          // Split the payload into a head and a tail
          const long buffer_size = m_fileBuffers[ftid].size();
          const long splitOffset = pagesize - buffer_size;
          long tailLength = sizeSum - pagesize;
          assert(splitOffset >= 0 && tailLength > 0);
          daqling::utilities::Binary head(m_payloads.frontPtr()->startingAddress(), splitOffset);
          daqling::utilities::Binary tail(static_cast<char *>(m_payloads.frontPtr()->startingAddress())
                  + splitOffset, tailLength);
          m_payloads.popFront();
          DEBUG(" -> head length: " << splitOffset << "; tail length: " << tailLength);

          // Write head into buffer and flush it
          m_fileBuffers[ftid] += head;
          assert(m_fileBuffers[ftid].size() <= pagesize);
          DEBUG(" -> " << m_fileBuffers[ftid].size() << " [Bytes] will be written.");
          m_fileStreams[ftid].write(static_cast<char *>(m_fileBuffers[ftid].startingAddress()),
                                    m_fileBuffers[ftid].size());
          m_bytes_sent += m_fileBuffers[ftid].size();

          // Flush the tail until it is small enough to fit in a page
          while (tailLength > pagesize) {
            daqling::utilities::Binary body(tail.startingAddress(), pagesize);
            m_fileStreams[ftid].write(static_cast<char *>(body.startingAddress()), body.size());
            m_bytes_sent += body.size();

            daqling::utilities::Binary nextTail(static_cast<char *>(tail.startingAddress())
                    + pagesize, tailLength - pagesize);
            tail = nextTail;
            tailLength = nextTail.size();
            DEBUG(" -> head of tail flushed; new tail length: " << tailLength);
          }

          m_fileBuffers[ftid] = tail;
          assert(m_fileBuffers[ftid].size() <= pagesize);
        } else {                           // We can safely extend the buffer.

          // This is fishy:
          m_fileBuffers[ftid] +=
              *(reinterpret_cast<daqling::utilities::Binary *>(m_payloads.frontPtr()));
          assert(m_fileBuffers[ftid].size() <= pagesize);
          m_payloads.popFront();

          // daqling::utilities::Binary frontPayload(0);
          // m_payloads.read( std::ref(frontPayload) );
          // m_fileBuffers[ftid] += frontPayload;
        }
      } else {
        std::this_thread::sleep_for(1ms);
      }
    }
  };
  m_fileWriters[1]->set_work(m_writeFunctors[1]);
}

void FileDataLoggerModule::write() { INFO(" Should write..."); }

bool FileDataLoggerModule::write(uint64_t, daqling::utilities::Binary&) {
  INFO(" Should write...");
  return false;
}

void FileDataLoggerModule::read() {}

void FileDataLoggerModule::shutdown() {}

void FileDataLoggerModule::monitor_runner() {
  while (m_run) {
    std::this_thread::sleep_for(1s);
    INFO("Write throughput: " << static_cast<double>(m_bytes_sent) / 1000000.0 << " MBytes/s");
    INFO("Size guess " << m_payloads.sizeGuess());
    m_bytes_sent = 0;
  }
}
