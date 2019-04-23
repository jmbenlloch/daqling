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
				//cout<<itr->first<<'\t'<<*itr->second<<'\n';
/*
				string s = "A";
				zmq::message_t message1(s.size());
    			memcpy (message1.data(), s.data(), s.size());
				bool rc = publisher.send (message1, ZMQ_SNDMORE);
*/
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
