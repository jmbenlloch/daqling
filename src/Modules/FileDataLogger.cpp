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

#include "Modules/FileDataLogger.hpp"
#include "Utilities/Logging.hpp"

#define __METHOD_NAME__ daqling::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daqling::utilities::className(__PRETTY_FUNCTION__)

using namespace std::chrono_literals;
namespace daqutils = daqling::utilities;

extern "C" FileDataLogger *create_object() { return new FileDataLogger; }

extern "C" void destroy_object(FileDataLogger *object) { delete object; }

FileDataLogger::FileDataLogger() : m_payloads{10000}, m_stopWriters{false} {
  INFO("FileDataLogger::FileDataLogger");

#warning RS -> Needs to be properly configured.
  // Set up static resources...
  m_writeBytes = 4 * daqutils::Constant::Kilo; // 4K buffer writes
  std::ios_base::sync_with_stdio(false);
  m_fileNames[1] = "/tmp/test.bin";
  m_fileStreams[1] = std::fstream(m_fileNames[1], std::ios::out | std::ios::binary);
  m_fileBuffers[1] = daqling::utilities::Binary(0);
  setup();
}

FileDataLogger::~FileDataLogger() { 
  INFO("FileDataLogger::~FileDataLogger"); 
  // Tear down resources...
  m_stopWriters.store(true);
  m_fileStreams[1].close();
}

void FileDataLogger::start() { 
  DAQProcess::start();
  INFO(__METHOD_NAME__ << " getState: " << getState());
}

void FileDataLogger::stop() { 
  DAQProcess::stop();
  INFO(__METHOD_NAME__ << " getState: " << this->getState());
}

void FileDataLogger::runner() {
  INFO(__METHOD_NAME__ << " Running...");
  //auto& cm = daqling::core::ConnectionManager::instance();
  while (m_run) {
    daqutils::Binary pl(0);
    while (!m_connections.get(1, std::ref(pl))) {
      std::this_thread::sleep_for(10ms);
    }
    m_payloads.write(pl);
    DEBUG(__METHOD_NAME__ << "Wrote data from channel 1...");
  }
  INFO(__METHOD_NAME__ << " Runner stopped");
}

#warning RS -> File rotation implementation is missing
#warning RS -> Hardcoded values should come from config.
void FileDataLogger::setup() {
  // Loop through sources from config and add a file writer for each sink.
  int tid = 1;
  m_fileWriters[tid] = std::make_unique<daqling::utilities::ReusableThread>(11111);
  m_writeFunctors[tid] = [&, tid]{
     int ftid = tid;
     INFO(__METHOD_NAME__ << " Spawning fileWriter for link: " << ftid);
     while( !m_stopWriters ){
       if ( m_payloads.sizeGuess() > 0 ) {
         DEBUG(__METHOD_NAME__ 
               << " SIZES: Queue pop.: " << m_payloads.sizeGuess() 
               << " loc.buff. size: " << m_fileBuffers[ftid].size() 
               << " payload size: " << m_payloads.frontPtr()->size());
         long sizeSum = long(m_fileBuffers[ftid].size()) + long(m_payloads.frontPtr()->size());
         if ( sizeSum > m_writeBytes ) { // Split needed.
           DEBUG(__METHOD_NAME__ << " Processing split.");
           long splitSize = sizeSum - m_writeBytes; // Calc split size
           long splitOffset = long(m_payloads.frontPtr()->size()) - long(splitSize); // Calc split offset
           DEBUG(" -> Sizes: | postPart: " << splitSize << " | For fillPart: " << splitOffset); 
           daqling::utilities::Binary fillPart(m_payloads.frontPtr()->startingAddress(), splitOffset);
           DEBUG(" -> filPart DONE.");
           daqling::utilities::Binary postPart(static_cast<char*>(m_payloads.frontPtr()->startingAddress()) + splitOffset, splitSize);
           DEBUG(" -> postPart DONE.");
           m_fileBuffers[ftid] += fillPart;
           DEBUG(" -> " << m_fileBuffers[ftid].size() << " [Bytes] will be written.");
           m_fileStreams[ftid].write(static_cast<char*>(m_fileBuffers[ftid].startingAddress()), m_fileBuffers[ftid].size()); // write
           m_fileBuffers[ftid] = postPart; // Reset buffer to postPart.
           m_payloads.popFront(); // Pop processed payload 
         } else { // We can safely extend the buffer.
           
           // This is fishy:
           m_fileBuffers[ftid] += *(reinterpret_cast<daqling::utilities::Binary*>( m_payloads.frontPtr() ));
           m_payloads.popFront();

           //daqling::utilities::Binary frontPayload(0);
           //m_payloads.read( std::ref(frontPayload) );
           //m_fileBuffers[ftid] += frontPayload;
 
         }
       }
       std::this_thread::sleep_for(10ms);
     }
  };
  m_fileWriters[1]->set_work(m_writeFunctors[1]);
}

void FileDataLogger::write() {
  INFO(__METHOD_NAME__ << " Should write...");
}

bool FileDataLogger::write(uint64_t keyId, daqling::utilities::Binary& payload) {
  INFO(__METHOD_NAME__ << " Should write...");
  return false;
}

void FileDataLogger::read() {}

void FileDataLogger::shutdown() {}

