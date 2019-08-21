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

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdint>
#include <functional>
#include "Binary.hpp"

using namespace daqling::utilities;

bool Binary::malloc(const size_t size) noexcept
{
  m_data = std::malloc(size);
  m_error = !m_data && size > 0;
  if (!m_error) {
      m_size = size;
  }
  return !m_error;
}

bool Binary::realloc(const size_t size) noexcept
{
  void* new_data = std::realloc(m_data, size);

  if (!new_data && size > 0) {
    m_error = true;
    return m_error;
  }

  m_data = new_data;
  m_size = size;

  return !m_error;
}

void* Binary::memcpy(void *dest, const void *src, const size_t n) noexcept
{
  assert(dest && src);

  // Ensure that memory areas do not overlap (undefined behavior; memmove(3) should be used instead)
  assert(std::invoke([src = reinterpret_cast<uintptr_t>(src), dest = reinterpret_cast<uintptr_t>(dest), n]() {
    const auto minimum_area = 2 * n,
      utilized_area = std::max(src, dest) + n - std::min(src, dest);

    return utilized_area >= minimum_area;
  }));

  return std::memcpy(dest, src, n);
}

Binary::Binary() noexcept
  : m_size{0}
{
  this->malloc(m_size);
}

Binary::Binary(const void* data, const size_t size) noexcept
  : m_size{size}
{
  if (!data) {
    // XXX: not an allocation error (invalid argument)
    m_error = true;
    return;
  }

  if(!this->malloc(m_size) || m_size == 0) {
    return;
  }

  this->memcpy(m_data, data, size);
}

Binary::Binary(const Binary& rhs) noexcept
  : m_size{rhs.m_size}, m_data{nullptr}
{
  if (!rhs.m_data) {
    m_error = true;
    return;
  }

  if (!this->malloc(m_size) || m_size == 0) {
    return;
  }

  this->memcpy(m_data, rhs.m_data, m_size);
}

Binary::Binary(Binary&& rhs) noexcept
  : m_size{std::exchange(rhs.m_size, 0)},
  m_data{std::exchange(rhs.m_data, nullptr)},
  m_error{std::exchange(rhs.m_error, false)}
{
}

Binary::~Binary() noexcept
{
  if (m_data) {
    std::free(m_data);
  }
}

Binary& Binary::operator=(const Binary& rhs) noexcept
{
  assert(!m_error);
  if (this == &rhs) {
    return *this;
  }

  if (m_data) {
    // Memory already allocated; reallocate.
    if (rhs.m_size > 0) {
      if (!this->realloc(rhs.m_size)) {
        return *this;
      }

      this->memcpy(m_data, rhs.m_data, rhs.m_size);
    } else {
      // No data in rhs; free this.
      std::free(std::exchange(m_data, nullptr));
    }
  } else {
    // No memory allocated; allocate.
    if (rhs.m_size > 0) {
      if (!this->malloc(rhs.m_size)) {
        return *this;
      }

      this->memcpy(m_data, rhs.m_data, rhs.m_size);
    }
  }

  m_size = rhs.m_size;
  return *this;
}

Binary& Binary::operator+=(const Binary& rhs) noexcept
{
  assert(!m_error);

  if (rhs.m_size > 0) {
    const size_t old_size = m_size;
    this->extend(rhs.m_size);
    if (m_error) {
      return *this;
    }

    this->memcpy(static_cast<char*>(m_data) + old_size, rhs.m_data, rhs.size());
  }

  return *this;
}

bool Binary::operator==(const Binary& rhs) const noexcept
{
  if (m_size != rhs.m_size) return false;
  if (m_size == 0) return true; // both are empty

  auto a = static_cast<char*>(m_data);
  auto b = static_cast<char*>(rhs.m_data);

  for (size_t i = 0; i < m_size; i++, a++, b++) {
    if (*a != *b) {
      return false;
    }
  }

  return true;
}

void Binary::extend(const size_t size) noexcept
{
  assert(!m_error);
  if (!m_data) {
    m_error = true;
    return;
  }

  this->realloc(m_size + size);
}

void Binary::resize(const size_t size) noexcept
{
  assert(!m_error);
  if (!m_data || size == m_size) {
    return;
  }

  this->realloc(size);
}
