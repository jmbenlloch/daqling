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
#include "Utilities/Binary.hpp"
#include "Utilities/Logging.hpp"


using namespace daqling::utilities;

Binary::Binary()
  : m_size( 0 )
  , m_data( 0 )
{
}


Binary::Binary( long initialSizeInBytes )
  : m_size( initialSizeInBytes )
{
  if ( m_size<0 ) // Fix Coverity-related bug #95349
    ERROR(__METHOD_NAME__  << " Cannot create a Binary with a negative size!");
  m_data = ::malloc( m_size ); // Fix Coverity NEGATIVE_RETURNS
  if ( (!m_data) && m_size>0 )
    ERROR(__METHOD_NAME__  << "::malloc failed!");
}

Binary::Binary( const void* data, long size )
  : m_size( size )
{
  if ( m_size<0 )
    ERROR(__METHOD_NAME__  << " Cannot copy a Binary with a negative size!");
  m_data = ::malloc( m_size ); // Fix Coverity NEGATIVE_RETURNS
  ::memcpy( startingAddress(), data, size );
}


Binary::~Binary()
{
  if ( m_data ) ::free( m_data );
}


const void*
Binary::startingAddress() const
{
  return m_data;
}


void*
Binary::startingAddress()
{
  return m_data;
}


const void* 
Binary::data() const
{
  if(!m_data)
    ERROR(" Binary data can't be accessed.");
  return startingAddress();
}

long
Binary::size() const
{
  return m_size;
}


void
Binary::extend( long additionalSizeInBytes )
{
  if ( additionalSizeInBytes<0 ) // Fix Coverity-related bug #95349
    ERROR(__METHOD_NAME__  << " Cannot extend by a negative size!");

  m_data = ::realloc( m_data, m_size + additionalSizeInBytes );
  m_size += additionalSizeInBytes;
  if ( (!m_data) && m_size>0 )
    ERROR(__METHOD_NAME__  << "::realloc failed!");
}


void
Binary::resize( long sizeInBytes )
{
  if ( sizeInBytes<0 ) // Fix Coverity-related bug #95349
    ERROR(__METHOD_NAME__  << " Cannot resize to a negative size!");

  if ( sizeInBytes != m_size )
  {
    m_data = ::realloc( m_data, sizeInBytes );
    m_size = sizeInBytes;
    if ( (!m_data) && m_size>0 )
      ERROR(__METHOD_NAME__  << "::realloc failed");

  }
}


Binary::Binary( const Binary& rhs )
  : m_size( rhs.m_size )
  , m_data( 0 )
{
  if ( m_size<0 ) // Fix Coverity-related bug #95349
    ERROR(__METHOD_NAME__  << " Cannot copy a Binary with a negative size!");

  if ( m_size > 0 )
  {
    m_data = ::malloc( m_size );
    if ( (!m_data) && m_size>0 )
      ERROR(__METHOD_NAME__  << "::malloc failed");

    m_data = ::memcpy( m_data, rhs.m_data, m_size );
  }
}


Binary&
Binary::operator=( const Binary& rhs )
{
  if ( this == &rhs ) return *this;  // Fix Coverity SELF_ASSIGN
  if ( rhs.m_size < 0 ) // Fix Coverity-related bug #95349
    ERROR(__METHOD_NAME__  << " Cannot assign a Binary with a negative size!");

  if ( m_data ) // check that old data is non-0 (instead of old m_size != 0)
  {
    if ( ( rhs.m_size > 0 )
         && rhs.m_data ) // unnecessary: try to silence Coverity MISSING_ASSIGN
    {
      m_data = ::realloc( m_data, rhs.m_size ); // realloc old (non-0) data
      if ( (!m_data) )
        ERROR(__METHOD_NAME__  << "::realloc failed");

      ::memcpy( m_data, rhs.m_data, rhs.m_size );
    }
    else
    {
      ::free( m_data ); // free and zero old (non-0) data
      m_data = 0;
    }
  }
  else // old data was 0
  {
    if ( ( rhs.m_size > 0 ) // Fix Coverity FORWARD_NULL (no memcpy if size<=0)
         && rhs.m_data ) // unnecessary: try to silence Coverity MISSING_ASSIGN
    {
      m_data = ::malloc( rhs.m_size ); // malloc new data (old was 0)
      if ( (!m_data) )
        ERROR(__METHOD_NAME__  << "::malloc failed");

      ::memcpy( m_data, rhs.m_data, rhs.m_size );
    }
    else
    {
      // nothing to do, old data was 0 and new data is still 0
    }
  }
  m_size = rhs.m_size; // finally set the size here
  return *this;
}


Binary&
Binary::operator+=( const Binary& rhs )
{
  long initialSize = m_size;
  this->extend( rhs.size() );
  ::memcpy( static_cast<char*>(m_data) + initialSize, rhs.m_data, rhs.size() );
  return *this;
}


bool
Binary::operator==( const Binary& rhs ) const
{
  if ( m_size != rhs.m_size ) return false;
  if ( m_size == 0 ) return true;
  const unsigned char* thisData = static_cast<const unsigned char*>(m_data);
  const unsigned char* rhsData = static_cast<const unsigned char*>(rhs.m_data);
  for ( long i = 0; i < m_size; ++i, ++thisData, ++rhsData )
    if ( *thisData != *rhsData )
      return false;
  return true;
}

