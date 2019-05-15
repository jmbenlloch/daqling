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

#include "Utilities/Logging.hpp"

#include <thread>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <atomic>

#include "zmq.hpp"

namespace daqling {
namespace core {

class Statistics{
public:

  
  class Metric {
    
  };


  Statistics(std::unique_ptr<zmq::socket_t>& statSock) : m_stat_socket{statSock} {
    m_interval = 1;
    m_stop_thread = false;
  }
  
  ~Statistics(){
    m_stop_thread = true;
    if(m_stat_thread.joinable()) m_stat_thread.join();
  }

  bool configure(unsigned interval) {
    m_interval = interval;
    return true; 
  }

  void start(){
    INFO("Start");
    m_stat_thread = std::thread(&Statistics::CheckStatistics,this);
  }

  void registerVariable(std::string name, std::atomic<int>* a) {
//    m_registered_metrics.insert( std::make_pair(name,a));
  }

  void registerCoreMetric(std::string name, std::atomic<size_t>* metric) {
    m_registered_metrics.insert( std::make_pair(name, metric) );
  }

private:
  // Thread control
  std::thread m_stat_thread;
  std::atomic<bool> m_stop_thread;
 
  // Publish socket ref for stats
  std::unique_ptr<zmq::socket_t>& m_stat_socket;

  // Config
  unsigned m_interval;
  std::map<std::string, size_t> m_core_metrics;
  std::map<std::string, std::atomic<size_t>*> m_registered_metrics;

  // Runner
  void CheckStatistics() {
    INFO(" Statistics thread about to spawn...");
//    zmq::context_t context(1);
//    zmq::socket_t publisher(context, ZMQ_PUB);
//    publisher.bind("tcp://*:5556");


    while(!m_stop_thread){
      std::this_thread::sleep_for( std::chrono::seconds(m_interval) );
      size_t qSize = m_registered_metrics["CHN0-QueueSizeGuess"]->load();
      size_t newMsgs = m_registered_metrics["CHN0-NumMessages"]->exchange(0);
      DEBUG( " -------->>>>> STAT before publishing: qSize - " << qSize);
      DEBUG( " -------->>>>> STAT before publishing: newMessages in last " << m_interval << " seconds - " << newMsgs); 
      std::ostringstream str1;
      std::ostringstream str2;
      str1 << "CHN0-QueueSizeGuess: " << qSize;
      str2 << "CHN0-NumMessages: " << newMsgs;
      std::string msg1 = str1.str(); 
      std::string msg2 = str2.str();
      DEBUG( " publishing statistics messages: " << msg1 << " msg2: " << msg2);
      zmq::message_t message1(msg1.size());
      zmq::message_t message2(msg2.size());
      memcpy (message1.data(), msg1.data(), msg1.size());
      memcpy (message2.data(), msg2.data(), msg2.size());
      bool rc = m_stat_socket->send(message1);
      if (rc) { DEBUG("SUCCESS"); }
      rc = m_stat_socket->send(message2);
      if (rc) { DEBUG("SUCCESS"); }
  /*    std::map<std::string,std::atomic<size_t>*>::iterator itr;
      for(itr = m_registered_metrics.begin(); itr != m_registered_metrics.end(); ++itr) {
        std::string msg = itr->first + ": " + std::to_string(*itr->second->load());
	zmq::message_t message(msg.size());
        memcpy (message.data(), msg.data(), msg.size());
        INFO(" MSG " << msg);
	bool rc = publisher.send(message);
	if(rc) { 
          INFO("Success");
        } else {
          WARNING("No success???");
        }
      }
  */ 
    }

  } // CheckStatistics


};

}
}

