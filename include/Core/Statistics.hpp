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
#include <ctime>
#include <numeric>
#include "zmq.hpp"

namespace daqling {
namespace core {


enum metric_type {LAST_VALUE, ACCUMULATE, AVERAGE, RATE};
enum variable_type {FLOAT, INT, DOUBLE, BOOL};


class Metric_base {
    public:
    Metric_base(std::string name, metric_type mtype, variable_type vtype, float delta_t) : m_name(name), m_mtype(mtype), m_vtype(vtype), m_delta_t(delta_t) {
        m_timestamp = std::time(nullptr);
    }
    ~Metric_base(){}

    std::string m_name;   
    std::time_t m_timestamp;
    metric_type m_mtype; 
    variable_type m_vtype;
    float m_delta_t; //time between measurements in sec

};



template <class T, class U>
class Metric : public Metric_base {
    public:
    Metric(T* pointer, std::string name, metric_type mtype, variable_type vtype, float delta_t) : m_value(pointer), Metric_base(name, mtype, vtype, delta_t) {}
    ~Metric(){}
    std::vector<U> m_values;
    T* m_value;
        
};




class Statistics{
public:

 
    Statistics(std::unique_ptr<zmq::socket_t>& statSock) : m_stat_socket{statSock} {
        m_interval = 500;
        m_stop_thread = false;
    }
      
    ~Statistics(){
        for (auto &x : m_reg_metrics){
            delete x;
        }
        m_reg_metrics.clear();
        m_reg_metrics.shrink_to_fit();
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

    template <class T, class U>
    void registerVariable(T* pointer, std::string name, metric_type mtype, variable_type vtype, float delta_t = 5) {
        if(delta_t < m_interval/1000){
            delta_t = m_interval;
            INFO("delta_t parameter of registerVariable(...) function can not be smaller than m_interval! Setting delta_t to m_interval value.");
        }
        Metric<T, U>* metric = new Metric<T, U>(pointer, name, mtype, vtype, delta_t);
        m_reg_metrics.push_back(static_cast<Metric_base*>(metric));
    }

    template <class T, class U>
    void addValueToVector(Metric_base* m){
        Metric<T, U>* metric = static_cast<Metric<T, U>*>(m);
        std::cout<<"Curr value: "<<*(metric->m_value)<<std::endl;
        U value = *(metric->m_value);
        metric->m_values.push_back(value);
    }

    template <class T, class U>
    U getValue(Metric_base* m){
        Metric<T, U>* metric = static_cast<Metric<T, U>*>(m);
        if(metric->m_mtype == AVERAGE){
            U average = std::accumulate( metric->m_values.begin(), metric->m_values.end(), 0.0)/metric->m_values.size();
            metric->m_values.clear();
            metric->m_values.shrink_to_fit();
            return average;
        }
        else if(metric->m_mtype == LAST_VALUE)
            return *(metric->m_value);
        else
            return *(metric->m_value);
    }


    void registerVariable(void* metric) {
        m_reg_metrics_void.push_back(metric);
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
    std::vector<Metric_base*> m_reg_metrics;
    std::vector<void*> m_reg_metrics_void;

    // Runner
    void CheckStatistics() {
        INFO("Statistics thread about to spawn...");
//    zmq::context_t context(1);
//    zmq::socket_t publisher(context, ZMQ_PUB);
//    publisher.bind("tcp://*:5556");

    
    	while(!m_stop_thread){
      
            std::this_thread::sleep_for( std::chrono::milliseconds(m_interval) );
            for (auto &x : m_reg_metrics){
                std::cout<<x->m_name<<std::endl;
                std::cout<<x->m_delta_t<<std::endl;
                if(std::difftime(std::time(nullptr), x->m_timestamp) < x->m_delta_t){
                    if(x->m_mtype == AVERAGE){
                        switch(x->m_vtype){
                        case FLOAT:
                            addValueToVector<std::atomic<float>, float >(x);
                            std::cout<<"add to vector"<<std::endl;
                            break;
                        case INT:
                            addValueToVector<std::atomic<int>, int>(x);
                            break;
                        case DOUBLE:
                            addValueToVector<std::atomic<double>, double>(x);
                            break;
                        case BOOL:
                            addValueToVector<std::atomic<bool>, bool>(x);
                            break;
                        };

                    }

                }
                else{
                    switch(x->m_vtype){
                    case FLOAT:
                        std::cout<<getValue<std::atomic<float>, float>(x)<<std::endl;
                        break;
                    case INT:
                        addValueToVector<std::atomic<int>, int>(x);
                        break;
                    case DOUBLE:
                        addValueToVector<std::atomic<double>, double>(x);
                        break;
                    case BOOL:
                        addValueToVector<std::atomic<bool>, bool>(x);
                        break;
                    };




                }
                switch(x->m_vtype){
                case FLOAT:
                    std::cout<<*(static_cast<Metric<std::atomic<float>, float>*>(x)->m_value)<<std::endl;
                    break;
                };
            }

 /*     size_t qSize = m_registered_metrics["CHN0-QueueSizeGuess"]->load();
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
*/



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

