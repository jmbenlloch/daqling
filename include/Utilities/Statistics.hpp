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


#include <thread>
#include <iostream>
#include <vector>
#include <map>
#include <atomic>

#include "zmq.hpp"

using namespace std;

class Statistics{
public:
	std::atomic<bool> stop_thread;
	
	Statistics(){
		stop_thread = false;
	}

	~Statistics(){
		stop_thread = true;
		if(m_stat_thread.joinable()) m_stat_thread.join();
	}

	
	void Start(){
		cout<<"Start"<<'\n'; 
		m_stat_thread = std::thread(&Statistics::CheckStatistics,this);
	}

	void RegisterVariable(string name, std::atomic<int>* a){variables_map.insert(make_pair(name,a));}

private:
	thread m_stat_thread;
	map<string,std::atomic<int>*> variables_map;

	void CheckStatistics(){
		cout<<"Check statistics"<<'\n'; 
		zmq::context_t context(1);
		zmq::socket_t publisher(context, ZMQ_PUB);
		publisher.bind("tcp://*:5556");
		while(!stop_thread){
			this_thread::sleep_for( std::chrono::seconds(1) );
			map<string,std::atomic<int>*>::iterator itr;
			for(itr = variables_map.begin(); itr != variables_map.end(); ++itr) {
				string msg = itr->first + ": " + to_string(*itr->second);
				zmq::message_t message(msg.size());
    			memcpy (message.data(), msg.data(), msg.size());
				cout<<msg.data()<<endl;

				bool rc = publisher.send(message);
				if(rc)
					cout<<"Success"<<endl; 
			}
		}
	}

};
