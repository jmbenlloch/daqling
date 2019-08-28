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
#include <ctime>
#include <sstream>
/// \endcond

#include "FileDataWriterModule.hpp"
#include "Utils/Logging.hpp"

using namespace std::chrono_literals;
namespace daqutils = daqling::utilities;

std::ofstream FileDataWriterModule::FileGenerator::next()
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

  DEBUG("Next generated filename is: " << ss.str());

  return std::ofstream(ss.str(), std::ios::binary);
}

bool FileDataWriterModule::FileGenerator::yields_unique(const std::string &pattern)
{
  bool chid = false;
  bool number = false;

  for (auto c = pattern.cbegin(); c != pattern.cend(); c++) {
    /*
     * While %D can conceptually yield unique files it has a resolution of 1s;
     * if FileGenerator::next() is called too often files will be overwritten.
     */
    if (*c == '%' && c + 1 != pattern.cend()) {
      const auto nc = *(++c);
      if (nc == 'n')
        number = true;
      if (nc == 'c')
        chid = true;
    }
  }

  return chid && number;
}

FileDataWriterModule::FileDataWriterModule() : m_stopWriters{false}
{

  INFO(__METHOD_NAME__);

  /* #warning RS -> Needs to be properly configured. */
  // Set up static resources...
  std::ios_base::sync_with_stdio(false);
  setup();
}

FileDataWriterModule::~FileDataWriterModule()
{
  INFO(__METHOD_NAME__);
  // Tear down resources...
  m_stopWriters.store(true);
}

void FileDataWriterModule::start()
{
  DAQProcess::start();
  INFO(" getState: " << getState());

  m_monitor_thread = std::thread(&FileDataWriterModule::monitor_runner, this);

  if (m_statistics) {
    // Register statistical variables
    for (auto & [ chid, metrics ] : m_channelMetrics) {
      m_statistics->registerVariable<std::atomic<size_t>, size_t>(
        &metrics.bytes_written,
        fmt::format("DL_BytesWritten_chid{}", chid),
        daqling::core::metrics::RATE,
        daqling::core::metrics::SIZE);
      m_statistics->registerVariable<std::atomic<size_t>, size_t>(
        &metrics.payload_queue_size,
        fmt::format("DL_PayloadQueueSize_chid{}", chid),
        daqling::core::metrics::LAST_VALUE,
        daqling::core::metrics::SIZE);
      m_statistics->registerVariable<std::atomic<size_t>, size_t>(
        &metrics.payload_queue_bytes,
        fmt::format("DL_PayloadQueueBytes_chid{}", chid),
        daqling::core::metrics::LAST_VALUE,
        daqling::core::metrics::SIZE);
    }
  }
}

void FileDataWriterModule::stop()
{
  DAQProcess::stop();
  INFO(" getState: " << this->getState());
  if (m_monitor_thread.joinable()) {
    m_monitor_thread.join();
  }
}

void FileDataWriterModule::runner()
{
  DEBUG(" Running...");

  // Start the producer thread of each context
  for (auto & [ chid, ctx ] : m_channelContexts) {
    std::get<ThreadContext>(ctx).producer.set_work([&]() {
      auto &pq = std::get<PayloadQueue>(ctx);

      while (m_run) {
        daqutils::Binary pl;
        while (!m_connections.get(chid, std::ref(pl)) && m_run) {
          if (m_statistics) {
            m_channelMetrics.at(chid).payload_queue_size += pq.sizeGuess();
          }
          std::this_thread::sleep_for(1ms);
        }

        DEBUG(" Received " << pl.size() << "B payload on channel: " << chid);
        while (!pq.write(pl) && m_run)
          ; // try until successful append
        if (m_statistics) {
          m_channelMetrics.at(chid).payload_queue_bytes += pl.size();
        }
      }
    });
  }

  while (m_run) {
    std::this_thread::sleep_for(1ms);
  };

  DEBUG(" Runner stopped");
}

void FileDataWriterModule::flusher(const uint64_t chid,
                                   PayloadQueue &pq,
                                   const size_t max_buffer_size,
                                   FileGenerator fg) const
{
  size_t bytes_written = 0;
  std::ofstream out = fg.next();
  auto buffer = daqutils::Binary();

  const auto flush = [&](daqutils::Binary &data) {
    out.write(data.data<char *>(), static_cast<std::streamsize>(data.size()));
    if (out.fail()) {
      CRITICAL(" Write operation for channel " << chid << " of size " << data.size()
                                               << "B failed!");
      throw std::runtime_error("std::ofstream::fail()");
    }
    m_channelMetrics.at(chid).bytes_written += data.size();
    bytes_written += data.size();
    data = daqutils::Binary();
  };

  while (!m_stopWriters) {
    while (pq.isEmpty() && !m_stopWriters) { // wait until we have something to write
      std::this_thread::sleep_for(1ms);
    };
    if (m_stopWriters) {
      flush(buffer);
      return;
    }

    if (bytes_written + buffer.size() > m_max_filesize) { // Rotate output files
      INFO(" Rotating output files for channel " << chid);
      flush(buffer);
      out.flush();
      out.close();
      out = fg.next();
      bytes_written = 0;
    }

    auto payload = pq.frontPtr();

    if (payload->size() + buffer.size() <= max_buffer_size) {
      buffer += *payload;
    } else {
      DEBUG("Processing buffer split.");
      const size_t split_offset = max_buffer_size - buffer.size();
      size_t tail_len = buffer.size() + payload->size() - max_buffer_size;
      assert(tail_len > 0);

      // Split the payload into a head and a tail
      daqutils::Binary head(payload->data(), split_offset);
      daqutils::Binary tail(payload->data<char *>() + split_offset, tail_len);
      DEBUG(" -> head length: " << head.size() << "; tail length: " << tail.size());
      assert(head.size() + tail.size() == payload->size());

      buffer += head;
      flush(buffer);

      // Flush the tail until it is small enough to fit in the buffer
      while (tail_len > max_buffer_size) {
        daqutils::Binary body(tail.data(), max_buffer_size);
        daqutils::Binary next_tail(tail.data<char *>() + max_buffer_size,
                                   tail_len - max_buffer_size);
        assert(body.size() + next_tail.size() == tail.size());
        flush(body);

        tail = next_tail;
        tail_len = next_tail.size();
        DEBUG(" -> head of tail flushed; new tail length: " << tail_len);
      }

      buffer = std::move(tail);
      assert(buffer.size() <= max_buffer_size);
    }

    // We are done with the payload; destruct it.
    pq.popFront();
  }
}

void FileDataWriterModule::setup()
{
  // Read out required and optional configurations
  m_max_filesize =
    m_config.getConfig()["settings"].value("max_filesize", 1 * daqutils::Constant::Giga);
  const size_t buffer_size =
    m_config.getConfig()["settings"].value("buffer_size", 4 * daqutils::Constant::Kilo);
  m_channels = m_config.getConfig()["connections"]["receivers"].size();
  const std::string pattern = m_config.getConfig()["settings"]["filename_pattern"];
  INFO("Configuration:");
  INFO(" -> Maximum filesize: " << m_max_filesize << "B");
  INFO(" -> Buffer size: " << buffer_size << "B");
  INFO(" -> channels: " << m_channels);

  if (!FileGenerator::yields_unique(pattern)) {
    WARNING("Configured file name pattern '"
            << pattern
            << "' may not yield unique output file on rotation; your files may be silently "
               "overwritten. Ensure the pattern contains '\%c' and '\%n'.");
  }

  unsigned int threadid = 11111;       // XXX: magic
  constexpr size_t queue_size = 10000; // XXX: magic

  for (uint64_t chid = 1; chid <= m_channels; chid++) {
    // For each channel, construct a context of a payload queue, a consumer thread, and a producer
    // thread.
    std::array<unsigned int, 2> tids = {threadid++, threadid++};
    const auto & [ it, success ] =
      m_channelContexts.emplace(chid, std::forward_as_tuple(queue_size, std::move(tids)));
    assert(success);

    // Contruct variables for metrics
    m_channelMetrics[chid];

    // Start the context's consumer thread.
    std::get<ThreadContext>(it->second)
      .consumer.set_work(&FileDataWriterModule::flusher,
                         this,
                         it->first,
                         std::ref(std::get<PayloadQueue>(it->second)),
                         buffer_size,
                         FileGenerator(pattern, it->first));
  }
  assert(m_channelContexts.size() == m_channels);

  DEBUG("setup finished");
}

void FileDataWriterModule::write()
{
  INFO(" Should write...");
}

bool FileDataWriterModule::write(uint64_t, daqling::utilities::Binary &)
{
  INFO(" Should write...");
  return false;
}

void FileDataWriterModule::read() {}

void FileDataWriterModule::shutdown() {}

void FileDataWriterModule::monitor_runner()
{
  while (m_run) {
    std::this_thread::sleep_for(1s);
    // XXX: is this really "throughput"?
    for (auto & [ chid, metrics ] : m_channelMetrics) {
      INFO("Write throughput (channel "
           << chid << "): " << static_cast<double>(metrics.bytes_written) / 1000000 << " MBytes/s");
      if (!m_statistics) {
        // We have to reset the variable ourselves
        metrics.bytes_written = 0;
      }
    }
  }
}
