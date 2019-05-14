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

#ifndef DAQ_CORE_DATALOGGER_HPP_
#define DAQ_CORE_DATALOGGER_HPP_

#include "Core/Configuration.hpp"
#include "Utilities/Common.hpp"
#include "Utilities/DataStore.hpp"
#include "Utilities/Logging.hpp"

#define __METHOD_NAME__ daq::utilities::methodName(__PRETTY_FUNCTION__)
#define __CLASS_NAME__ daq::utilities::className(__PRETTY_FUNCTION__)

/*
 * ConnectionManager
 * Author: Roland.Sipos@cern.ch
 * Description: DataLogger interface with generic datastore
 * Date: March 2019
 */

class DataLogger {
 public:
  DataLogger() {}
  virtual ~DataLogger(){};

  virtual void setup() = 0;
  virtual void write() = 0;
  virtual void read() = 0;
  virtual bool write(uint64_t keyId, daq::utilities::Binary& payload) = 0;
  virtual void shutdown() = 0;

 protected:
  daq::utilities::DataStoreBase m_dataStore;
};

#endif /* DAQPROCESS_HPP_ */
