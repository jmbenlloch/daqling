#include "Utilities/Statistics.hpp"
#include <unistd.h>
#include <atomic> 
#include "zmq.hpp"

int main(int argc, char** argv)
{

	std::atomic<int> buffer_occupation;
	std::atomic<int> packets;
	buffer_occupation = 0;
	packets = 0;

	Statistics stat;
	stat.Start();
	cout<<"Called start"<<'\n'; 
	stat.RegisterVariable("Buffer occupation", &buffer_occupation);
	stat.RegisterVariable("Number of packets", &packets);
	while(1){
		usleep(500000);
		//std::cout<<"bla"<<std::endl;
		packets+=2;
		buffer_occupation+=3;
	}

	return 0;
}

