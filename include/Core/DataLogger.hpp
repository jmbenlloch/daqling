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

#ifndef DAQLING_CORE_DATALOGGER_HPP
#define DAQLING_CORE_DATALOGGER_HPP

#include "Core/Configuration.hpp"
#include "Utilities/DataStore.hpp"
#include "Utilities/Logging.hpp"


/*
 * DataLogger
 * Description: DataLogger interface with generic datastore
 * Date: March 2019
 */

namespace daqling {
namespace core {

class DataLogger {
 public:
  DataLogger() {}
  virtual ~DataLogger(){};

  virtual void setup() = 0;
  virtual void write() = 0;
  virtual void read() = 0;
  virtual bool write(uint64_t keyId, daqling::utilities::Binary& payload) = 0;
  virtual void shutdown() = 0;

 protected:
  daqling::utilities::DataStoreBase m_dataStore;
};

} // namespace core
} // namespace daqling

#endif // DAQLING_CORE_DATALOGGER_HPP
