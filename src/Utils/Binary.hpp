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
#include <type_traits>
#include <cctype>
#include <functional>
#include <algorithm>

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

    /// Returns the internally stored data
    template<typename T = void*>
    const T data() const noexcept
    {
        static_assert(std::is_pointer<T>(), "Type parameter must be a pointer type");
        return static_cast<T>(m_data);
    }

    /// Returns the internally stored data
    template<typename T = void*>
    T data() noexcept
    {
        static_assert(std::is_pointer<T>(), "Type parameter must be a pointer type");
        return static_cast<T>(m_data);
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

/// xxd(1)-like output representation of a `Binary`
inline std::ostream& operator<<(std::ostream &out, const daqling::utilities::Binary &rhs)
{
    for (size_t i = 0; i < rhs.size(); i++) {
        const bool newline_prefix = i % 16 == 0;
        const bool newline = (i + 1) % 16 == 0;
        const bool seperate = (i + 1) % 2 == 0;
        const bool last_byte = i == rhs.size() - 1;

        auto c = rhs.data<uint8_t*>() + i;

        if (newline_prefix) {
            // Print offset
            out << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
        }

        // Print a line of byte pairs
        out << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(*c)
            << (seperate ? " " : "");

        if (newline || last_byte) {
            const auto str = std::invoke([c, i]() {
                std::locale loc("C");
                std::string str(c - i % 16, c);

                // Replace unprintable characters with a '.'
                std::replace_if(str.begin(), str.end(), [&loc](auto c) { return !std::isprint(c, loc); }, '.');

                return std::move(str);
            });

            // Print the character string representation of the byte line
            const size_t blanks = 40 // byte line length
                - (i % 16 + 1) * 2 // space taken up by byte pairs
                - (i % 16) / 2; // space taken up by spacing between byte pairs
            out << std::string(blanks, ' ') << str << '\n';
        }
    }

    return out;
}

#endif // DAQ_UTILITIES_BINARY_HPP
