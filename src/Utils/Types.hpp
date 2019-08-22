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

#ifndef DAQLING_UTILITIES_TYPES_HPP
#define DAQLING_UTILITIES_TYPES_HPP

/*
 * Types
 * Description:
 *   Collection of custom types used in the DAQ.
 * Date: May 2018
 */

namespace daqling {
  namespace utilities {

    typedef std::uint64_t timestamp_t;

    static const timestamp_t ns = 1;
    static const timestamp_t us = 1000 * ns;
    static const timestamp_t ms = 1000 * us;
    static const timestamp_t s = 1000 * ms;

    typedef enum { UNKNOWN = 0, BINARY, INMEMORY, CASSANDRA, REST } EBackendType;

  } // namespace utilities
} // namespace daqling

#endif // DAQLING_UTILITIES_TYPES_HPP
