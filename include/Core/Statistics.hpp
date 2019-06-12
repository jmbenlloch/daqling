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

#ifndef STATISTICS_HPP
#define STATISTICS_HPP


#include "Utilities/Logging.hpp"
#include "Core/Metric.hpp"

#include <thread>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <atomic>
#include <ctime>
#include <numeric>
#include "zmq.hpp"

namespace daqling {
namespace core {



class Statistics{
public:

 
  Statistics(std::unique_ptr<zmq::socket_t>& statSock, unsigned interval=500);
  
  ~Statistics();

  bool configure(unsigned interval);

  void start();
  
  void registerCoreMetric(std::string name, std::atomic<size_t>* metric);

  template <class T, class U>
  void registerVariable(T* pointer, std::string name, metric_type mtype, variable_type vtype, float delta_t=1) {
    if(delta_t < m_interval/1000){
      delta_t = m_interval;
      INFO("delta_t parameter of registerVariable(...) function can not be smaller than m_interval! Setting delta_t to m_interval value.");
    }
    //Metric<T, U>* metric = new Metric<T, U>(pointer, name, mtype, vtype, delta_t);
    //m_reg_metrics.push_back(static_cast<Metric_base*>(metric));
    std::shared_ptr<Metric<T, U> > metric(new Metric<T, U>(pointer, name, mtype, vtype, delta_t));
    std::shared_ptr<Metric_base> metric_base = std::dynamic_pointer_cast<Metric_base>(metric);
    m_reg_metrics.push_back(metric_base);
  }



  template <class T, class U>
  void accumulateValue(Metric_base* m){
    Metric<T, U>* metric = static_cast<Metric<T, U>*>(m);
    U value = *(metric->m_metrics_ptr);
    metric->m_values.push_back(value);
  }



  template <class T, class U>
  bool publishValue(Metric_base* m){
    U value = 0;
    Metric<T, U>* metric = static_cast<Metric<T, U>*>(m);
    if(metric->m_mtype == AVERAGE){
      U average = std::accumulate( metric->m_values.begin(), metric->m_values.end(), 0.0)/metric->m_values.size();
      metric->m_values.clear();
      metric->m_values.shrink_to_fit();
      value = average;
    }
    else if(metric->m_mtype == LAST_VALUE){
      value = *(metric->m_metrics_ptr);
      metric->m_values.clear();
      metric->m_values.shrink_to_fit();
      metric->m_values.push_back(value);
    }
    else if(metric->m_mtype == RATE){
      value = *(metric->m_metrics_ptr);
      U last_value = 0;
      if(metric->m_values.size() == 1){
        last_value = metric->m_values[0];
      }
      metric->m_values.clear();
      metric->m_values.shrink_to_fit();
      metric->m_values.push_back(value);
      if(std::difftime(std::time(nullptr), metric->m_timestamp) != 0)
        value = (value - last_value)/(U)std::difftime(std::time(nullptr), metric->m_timestamp);
      else{
        WARNING("Too short time interval to calculate RATE! Extend delta_t parameter of your metric");
        return false;
      }
    }

    metric->m_timestamp = std::time(nullptr);
    std::ostringstream msg;
    msg<<metric->m_name<<": "<<value;
    std::cout<<value;
    std::cout<<msg.str();
    //std::string msg = metric->m_name + ": " + std::to_string(value);
    zmq::message_t message(msg.str().size());
    memcpy (message.data(), msg.str().data(), msg.str().size());
    INFO(" MSG " << msg.str());
    bool rc = m_stat_socket->send(message);
    if(!rc)
      WARNING("Failed to publish metric: " << metric->m_name);
    return rc;


  }


  private:

  // Thread control
  std::thread m_stat_thread;
  std::atomic<bool> m_stop_thread;
 
  // Publish socket ref for stats
  std::unique_ptr<zmq::socket_t>& m_stat_socket;

  // Config
  unsigned m_interval;
  std::map<std::string,  std::atomic<size_t>*> m_registered_metrics;
  //std::vector<Metric_base*> m_reg_metrics;
  std::vector<std::shared_ptr<Metric_base> > m_reg_metrics;

  // Runner
  void CheckStatistics();

};

} // namespace core
} // namespace daqling

#endif // STATISTICS_HPP
