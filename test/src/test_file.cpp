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
#include "Core/ConnectionManager.hpp"

using namespace daq;

int
main(int argc, char** argv)
{

  INFO("WOOF WOOF");
  WARNING("Ugh!" << 12345 << "bof bof" << '\n');

  INFO("Testing ConnectionManager.hpp");
  daq::core::ConnectionManager<int, int>& cm = daq::core::ConnectionManager<int, int>::instance();
  

  ERROR("About to die...");
  return 0;
}

