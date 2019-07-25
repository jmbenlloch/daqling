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
#include <functional>
/// \endcond

#include "FileDataLoggerModule.hpp"
#include "Utils/Logging.hpp"


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

/* #warning RS -> Needs to be properly configured. */
  // Set up static resources...
  std::ios_base::sync_with_stdio(false);
  m_fileStreams[0] = create_ofile(m_filenum++);
  setup();
}

FileDataLoggerModule::~FileDataLoggerModule() {
  INFO(__METHOD_NAME__);
  // Tear down resources...
  m_stopWriters.store(true);
  m_fileStreams[0].close();
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

#warning RS -> proper termination not implemented
void FileDataLogger::flusher()
{
  long bytes_written = 0;

  // TODO: impl terminate
  while (!m_stopWriters) {
    while (m_payloads.isEmpty()); // wait until we have something to write

    if (bytes_written > m_max_filesize) { // Rotate output files
      INFO(" Rotating output files");
      m_fileStreams[0].flush();
      m_fileStreams[0].close();
      m_fileStreams[0] = create_ofile(m_filenum++);
      bytes_written = 0;
    }

    auto payload = m_payloads.frontPtr();
    m_payloads.popFront();
    m_fileStreams[0].write(static_cast<char *>(payload->startingAddress()), payload->size());
    m_bytes_sent += payload->size();
    bytes_written += payload->size();
  }
}

#warning RS -> Hardcoded values should come from config.
void FileDataLogger::setup() {
  m_max_filesize = std::invoke([this]() -> long {
    try {
      return std::stoi(std::string(m_config.getConfig()["settings"]["max_filesize"]));
    } catch (const nlohmann::json::exception&) {
      return 1 * daqutils::Constant::Giga;
    }
  });

  // Loop through sources from config and add a file writer for each sink.
  const int tid = 0;
  int threadid = 11111;

  // Construct and start flusher
  m_fileWriters[tid] = std::make_unique<daqling::utilities::ReusableThread>(threadid);
  m_fileWriters[tid]->set_work(std::bind(&FileDataLogger::flusher, this));
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
