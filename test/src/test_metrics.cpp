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
	stat.RegisterVariable("BufferOccupation", &buffer_occupation);
	stat.RegisterVariable("NumberOfPackets", &packets);
	while(1){
		usleep(500000);
		//std::cout<<"bla"<<std::endl;
		packets+=2;
		buffer_occupation+=3;
	}

	return 0;
}

