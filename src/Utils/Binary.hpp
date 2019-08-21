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

#ifndef DAQLING_UTILITIES_BINARY_HPP
#define DAQLING_UTILITIES_BINARY_HPP

/*
 * Binary
 * Description:
 *   A convenient void* wrapper, representing dynamically sized, continous memory area.
 */

#include <iostream>
#include <iomanip>

namespace daqling::utilities {

  class Binary
  {

  public:

    /// Default Constructor. Creates an empty BLOB
    Binary() noexcept;

    // Constructor initializing a BLOB with `size` bytes from `data`
    explicit Binary(const void* data, const size_t size) noexcept;

    /// Destructor. Frees internally allocated memory, if any
    ~Binary() noexcept;

    /// Copy constructor
    explicit Binary(const Binary& rhs) noexcept;

    /// Move constructor
    explicit Binary(Binary&& rhs) noexcept;

    /// Assignment operator
    Binary& operator=(const Binary& rhs) noexcept;

    /// Appends the data of another blob
    Binary& operator+=(const Binary& rhs) noexcept;

    /// Equal operator. Compares the contents of the binary blocks
    bool operator==(const Binary& rhs) const noexcept;

    /// Comparison operator
    inline bool operator!=(const Binary& rhs) const noexcept
    {
        return !this->operator==(rhs);
    }

    // TODO: make template
    /// Returns the internally stored data
    const void* data() const noexcept
    {
        return m_data;
    }

    /// Returns the internally stored data
    void* data() noexcept
    {
        return m_data;
    }

    /// Current size of the blob
    size_t size() const noexcept
    {
        return m_size;
    }

    /// Returns whether or not the Binary is in a usable state or if an error occured
    [[nodiscard]]
    bool good() const noexcept
    {
        return !m_error;
    }

  private:
    /// Extends the BLOB by `size` additional bytes
    void extend(const size_t size) noexcept;

    /// Resizes a BLOB to `size` total bytes
    void resize(const size_t size) noexcept;

    bool malloc(const size_t size) noexcept;
    bool realloc(const size_t size) noexcept;
    void* memcpy(void *dest, const void *src, const size_t n) noexcept;

    /// The current size of the BLOB
    size_t m_size;

    /// The BLOB data buffer
    void* m_data;

    /// Allocation error flag
    bool m_error;
  };

} // namespace daqling::utilities

inline std::ostream& operator<<(std::ostream& out, const daqling::utilities::Binary& rhs)
{
  for (size_t i = 0; i < rhs.size(); i++) {
    std::cout << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(*(static_cast<const unsigned char*>(rhs.data()) + i)) << std::dec;
    if (i % 4 == 3)
    {
      if (i % 16 == 15)
      {
        std::cout << "\n";
      }
      else
      {
        std::cout << " ";
      }
    }
  }
  std::cout << "\n";
  return out;
}

#endif // DAQ_UTILITIES_BINARY_HPP

