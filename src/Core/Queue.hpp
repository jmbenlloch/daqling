/**
 * Copyright (C) 2019-2021 CERN
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

#pragma once
#include "Common/DataType.hpp"
#include "Utils/Binary.hpp"
namespace daqling {
namespace core {
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class Queue {
public:
  // Virtual destructor
  virtual ~Queue() = default;

  virtual bool read(DataType &) = 0;
  virtual bool write(DataType &) = 0;
  virtual bool sleep_read(DataType & /*bin*/);
  virtual bool sleep_write(DataType & /*bin*/);
  virtual uint sizeGuess() = 0;
  virtual uint capacity() = 0;
  virtual void set_sleep_duration(uint ms);

protected:
  uint m_sleep_duration{};
};
} // namespace core
} // namespace daqling
