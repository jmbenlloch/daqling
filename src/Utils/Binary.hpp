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
 *   A really nice void* wrapper from CORAL
 *   https://twiki.cern.ch/twiki/bin/view/Persistency/Coral 
 * Date: May 2018
*/

#include <iostream>
#include <iomanip>

namespace daqling {
namespace utilities {

  class Binary
  {

  public:

    /// Default Constructor. Creates an empty BLOB
    Binary();

    /// Constructor initializing a BLOB with initialSize bytes
    explicit Binary( size_t initialSizeInBytes );

    explicit Binary( const void* data, size_t size );

    /// Destructor. Frees internally allocated memory
    ~Binary();

    /// Copy constructor
    Binary( const Binary& rhs );

    /// Assignment operator
    Binary& operator=( const Binary& rhs );

    /// Appends the data of another blob
    Binary& operator+=( const Binary& rhs );

    /// Equal operator. Compares the contents of the binary blocks
    bool operator==( const Binary& rhs ) const;

    /// Comparison operator
    bool operator!=( const Binary& rhs ) const;

    /// Returns the starting address of the BLOB
    const void* startingAddress() const;

    /// Returns the starting address of the BLOB
    void* startingAddress();

    const void* data() const;

    /// Current size of the blob
    size_t size() const;

    /// Extends the BLOB by additionalSizeInBytes
    void extend( size_t additionalSizeInBytes );

    /// Resizes a BLOB to sizeInBytes
    void resize( size_t sizeInBytes );

  private:

    /// The current size of the BLOB
    size_t m_size;

    /// The BLOB data buffer
    void* m_data;

  };

} // namespace utilities
} // namespace daqling

// Inline methods
inline bool
daqling::utilities::Binary::operator!=( const Binary& rhs ) const
{
  return ( ! ( this->operator==( rhs ) ) );
}

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

