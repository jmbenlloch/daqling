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

#include "FileDataLoggerModule.hpp"
#include "Utils/Logging.hpp"


using namespace std::chrono_literals;
namespace daqutils = daqling::utilities;

extern "C" FileDataLogger *create_object() { return new FileDataLogger; }

extern "C" void destroy_object(FileDataLogger *object) { delete object; }


std::ofstream FileDataLogger::FileGenerator::next()
{

  const auto handle_arg = [this](char c) -> std::string {
    switch (c) {
      case 'D': // Full date in YYYY-MM-DD-HH:MM:SS (ISO 8601) format
      {
        std::time_t t = std::time(nullptr);
        char tstr[32];
        if (!std::strftime(tstr, sizeof(tstr), "%F-%T", std::localtime(&t))) {
          throw std::runtime_error("Failed to format timestamp");
        }
        return std::string(tstr);
      }
      case 'n': // The nth generated output (equals the number of times called `next()`, minus 1)
        return std::to_string(m_filenum++);
      case 'c': // The channel id
        return std::to_string(m_chid);
      default:
        std::stringstream ss;
        ss << "Unknown output file argument '" << c << "'";
        throw std::runtime_error(ss.str());
    }
  };

  // Append every character until we hit a '%' (control character),
  // where the next character denotes an argument.
  std::stringstream ss;
  for (auto c = m_pattern.cbegin(); c != m_pattern.cend(); c++) {
    if (*c == '%' && c + 1 != m_pattern.cend()) {
      ss << handle_arg(*(++c));
    } else {
      ss << *c;
    }
  }

  return std::ofstream(ss.str(), std::ios::binary);
}

FileDataLogger::FileDataLogger()
  : m_stopWriters{false}, m_bytes_sent{0} {

  INFO(__METHOD_NAME__);

/* #warning RS -> Needs to be properly configured. */
  // Set up static resources...
  std::ios_base::sync_with_stdio(false);
  setup();
}

FileDataLoggerModule::~FileDataLoggerModule() {
  INFO(__METHOD_NAME__);
  // Tear down resources...
  m_stopWriters.store(true);
}

void FileDataLoggerModule::start() {
  DAQProcess::start();
  INFO(" getState: " << getState());
  m_monitor_thread = std::thread(&FileDataLogger::monitor_runner, this);
}

void FileDataLoggerModule::stop() {
  DAQProcess::stop();
  INFO(" getState: " << this->getState());
  m_monitor_thread.join();
  INFO("Joined successfully monitor thread");
}

void FileDataLoggerModule::runner() {
  INFO(" Running...");

  // Start the producer thread of each context
  for (auto &[chid, ctx] : m_channelContexts) {
    std::get<ThreadContext>(ctx).producer.set_work([&]() {
      auto &pq = std::get<PayloadQueue>(ctx);

      while (m_run) {
        daqutils::Binary pl(0);
        while (!m_connections.get(chid, std::ref(pl)) && m_run) {
          std::this_thread::sleep_for(1ms);
        }

        DEBUG(" Received " << pl.size() << "B payload on channel: " << chid);
        while (!pq.write(pl) && m_run); // try until successful append
      }
    });
  }

  while (m_run);

  INFO(" Runner stopped");
}

void FileDataLogger::flusher(PayloadQueue &pq, FileGenerator &&fg, const uint64_t chid) const
{
  long bytes_written = 0;
  std::ofstream out = fg.next();

  while (!m_stopWriters) {
    while (pq.isEmpty() && !m_stopWriters); // wait until we have something to write
    if (m_stopWriters) return;

    if (bytes_written > m_max_filesize) { // Rotate output files
      INFO(" Rotating output files for channel " << chid);
      out.flush();
      out.close();
      out = fg.next();
      bytes_written = 0;
    }

    auto payload = pq.frontPtr();
    pq.popFront();
    out.write(static_cast<char *>(payload->startingAddress()), payload->size());
    m_bytes_sent += payload->size();
    bytes_written += payload->size();
  }
}

void FileDataLogger::setup() {
  // Read out required and optional configurations
  m_max_filesize = m_config.getConfig()["settings"].value("max_filesize", 1 * daqutils::Constant::Giga);
  m_channels = m_config.getConfig()["connections"]["receivers"].size();
  const std::string pattern = m_config.getConfig()["settings"]["filename_pattern"];

  int threadid = 11111; // XXX: magic
  constexpr size_t queue_size = 10000; // XXX: magic

  for (uint64_t chid = 1; chid <= m_channels; chid++) {
    // For each channel, construct a context of a payload queue, a consumer thread, and a producer thread.
    std::array<int, 2> tids = {threadid++, threadid++};
    const auto& [it, success] = m_channelContexts.emplace(chid, std::forward_as_tuple(queue_size, std::move(tids)));
    assert(success);

    // Start the context's consumer thread.
    std::get<ThreadContext>(it->second).consumer.set_work([this, pattern, it]() {
      return flusher(std::get<PayloadQueue>(it->second), FileGenerator(pattern, it->first), it->first);
    });
  }
  assert(m_channelContexts.size() == m_channels);
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
    INFO("Write throughput: " << (double)m_bytes_sent / double(1000000) << " MBytes/s");
    m_bytes_sent = 0;
  }
}
