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
#include <ctime>
/// \endcond

#include "Modules/FileDataLogger.hpp"
#include "Utilities/Logging.hpp"


using namespace std::chrono_literals;
namespace daqutils = daqling::utilities;

extern "C" FileDataLogger *create_object() { return new FileDataLogger; }

extern "C" void destroy_object(FileDataLogger *object) { delete object; }

static std::ofstream create_ofile(int n)
{
    std::stringstream ss;
    std::time_t t = std::time(nullptr);

    char tstr[128];
    if (!std::strftime(tstr, sizeof(tstr), "%F-%T", std::localtime(&t))) {
        throw std::runtime_error("Failed to format timestamp");
    }

    ss << "/home/vsoneste/test-" << tstr << "." << n << ".bin";
    return std::ofstream(ss.str(), std::ios::binary);
}

FileDataLogger::FileDataLogger() : m_payloads{10000}, m_stopWriters{false}, m_bytes_sent{0} {
  INFO(__METHOD_NAME__);

#warning RS -> Needs to be properly configured.
  // Set up static resources...
  std::ios_base::sync_with_stdio(false);

  m_fileStreams[0] = create_ofile(m_filenum++);
  m_fileStreams[1] = create_ofile(m_filenum++);
  assert(m_fileStreams[0].is_open() && m_fileStreams[1].is_open());

  m_fileBuffers[0] = daqling::utilities::Binary(0);
  setup();
}

FileDataLogger::~FileDataLogger() {
  INFO(__METHOD_NAME__);
  // Tear down resources...
  m_stopWriters.store(true);
  m_fileStreams[0].close();
  m_fileStreams[1].close();
}

void FileDataLogger::start() {
  DAQProcess::start();
  INFO(" getState: " << getState());
  m_monitor_thread = std::make_unique<std::thread>(&FileDataLogger::monitor_runner, this);
}

void FileDataLogger::stop() {
  DAQProcess::stop();
  INFO(" getState: " << this->getState());
  m_monitor_thread->join();
  INFO("Joined successfully monitor thread");
}

void FileDataLogger::runner() {
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

#warning RS -> Hardcoded values should come from config.
void FileDataLogger::setup() {
  const long pagesize = std::invoke([this]() -> long {
    try {
      return std::stoi(std::string(m_config.getConfig()["settings"]["pagesize"]));
    } catch (const nlohmann::json::exception&) {
      return 4 * daqutils::Constant::Kilo; // 4K buffer
    }
  });
  const long max_filesize = std::invoke([this]() -> long {
    try {
      return std::stoi(std::string(m_config.getConfig()["settings"]["max_filesize"]));
    } catch (const nlohmann::json::exception&) {
      return 1 * daqutils::Constant::Giga;
    }
  });

  // Loop through sources from config and add a file writer for each sink.
  const int tid = 0;
  m_fileWriters[tid] = std::make_unique<daqling::utilities::ReusableThread>(11111);
  m_writeFunctors[tid] = [&, tid, pagesize] {
    const long max_filesize = 1 * daqutils::Constant::Giga;
    long bytes_written = 0;
    int ftid = tid;

    INFO(" Spawning fileWriter for link: " << ftid);
    while (!m_stopWriters) {
      if (m_payloads.sizeGuess() > 0) {
        if (bytes_written > max_filesize) { // rotate output file
          INFO(" File rotation");
          m_fileStreams[0].flush();
          m_fileStreams[0] = std::move(m_fileStreams[1]);
          m_fileStreams[1] = create_ofile(m_filenum++);
          assert(m_fileStreams[1].is_open());

          bytes_written = 0;
        }

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
          bytes_written += m_fileBuffers[ftid].size();

          // Flush the tail until it is small enough to fit in a page
          while (tailLength > pagesize) {
            daqling::utilities::Binary body(tail.startingAddress(), pagesize);
            m_fileStreams[ftid].write(static_cast<char *>(body.startingAddress()), body.size());
            m_bytes_sent += body.size();
            bytes_written += m_fileBuffers[ftid].size();

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
  m_fileWriters[tid]->set_work(m_writeFunctors[tid]);
}

void FileDataLogger::write() { INFO(" Should write..."); }

bool FileDataLogger::write(uint64_t keyId, daqling::utilities::Binary &payload) {
  INFO(" Should write...");
  return false;
}

void FileDataLogger::read() {}

void FileDataLogger::shutdown() {}

void FileDataLogger::monitor_runner() {
  while (m_run) {
    std::this_thread::sleep_for(1s);
    INFO("Write throughput: " << (double)m_bytes_sent / double(1000000) << " MBytes/s");
    INFO("Size guess " << m_payloads.sizeGuess());
    m_bytes_sent = 0;
  }
}