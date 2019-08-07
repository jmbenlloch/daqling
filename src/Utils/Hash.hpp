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

#ifndef DAQLING_UTILITIES_HASH_HPP
#define DAQLING_UTILITIES_HASH_HPP

#include "Logging.hpp"
#include "Binary.hpp"

#include <openssl/sha.h>

/*
 * Hash
 * Description: 
 *   Hash utils from CondDB
 * Date: March 2019
*/
namespace daqling {

  namespace persistency {

    typedef std::string Hash;
    static constexpr unsigned int HASH_SIZE = 40;

    inline Hash makeHash( const std::string& objectType, const daqling::utilities::Binary& data ){
      SHA_CTX ctx;
      if( !SHA1_Init( &ctx ) ){
        ERROR("daqling::persistency::makeHash: SHA1 initialization error.");
      }
      if( !SHA1_Update( &ctx, objectType.c_str(), objectType.size() ) ){
        ERROR("daqling::persistency::makeHash: SHA1 processing error (1).");
      }
      if( !SHA1_Update( &ctx, data.data(), data.size() ) ){
        ERROR("daqling::persistency::makeHash: SHA1 processing error (2).");
      }
      unsigned char hash[SHA_DIGEST_LENGTH];
      if( !SHA1_Final(hash, &ctx) ){
        ERROR("daqling::persistency::makeHash: SHA1 finalization error.");
      }
  
      char tmp[SHA_DIGEST_LENGTH*2+1];
      // re-write bytes in hex
      for (unsigned int i = 0; i < 20; i++) {
        ::sprintf(&tmp[i * 2], "%02x", hash[i]);
      }
      tmp[20*2] = 0;
      return tmp;
    }

  } // namespace persistency

} // namespace daqling

#endif // DAQLING_UTILITIES_HASH_HPP

