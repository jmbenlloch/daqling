#ifndef DAQ_MODULES_CASSANDRADATALOGGER_HPP_
#define DAQ_MODULES_CASSANDRADATALOGGER_HPP_

#include <iostream>

#include "core/DAQProcess.hpp"
#include "core/DataLogger.hpp"

#include "utilities/ChunkedStorage.hpp"
#include "utilities/Binary.hpp"

#include <cassandra.h>

/*
 * CassandraDataLogger
 * Author: Roland.Sipos@cern.ch
 * Description: Data logger with Cassandra persistency layer
 *   Heavily relies on the CondDB payload chunked storage.
 * Date: November 2017
*/
class CassandraDataLogger : public DAQProcess, public DataLogger
{

  class CassandraChunkedStorageProvider : public daq::persistency::ChunkedStorageProvider { 
    public:
      explicit CassandraChunkedStorageProvider(...) {
      // Any init?
      }     

      virtual ~CassandraChunkedStorageProvider() { };
      bool exists();
      void create();
      const size_t writeChunk(const std::string& objectName, int chunkId, const std::pair<const void*, size_t>& data, int ttl) const { return 0; }
      bool readChunk(const std::string& objectName, int chunkId, size_t split, void*& blobPtr) const { return false; };
      const int getDefaultChunkSize() { return 0; }
      void deleteObject(const std::string& objectName, int chunkCount) const { }
      void writeMetadata(const std::string& objectName, const daq::persistency::ObjectMetadata& attr) const { }
      const daq::persistency::ObjectMetadata readMetadata(const std::string& objectName) const { 
        daq::persistency::ObjectMetadata attributes;
        return attributes;
      } 

    private:
      const std::string M_NAME = "chunk";
  };

  public:
    CassandraDataLogger();
    ~CassandraDataLogger();

    const std::string KEYSPACE_NAME = "rd51daq";
    

    void start();
    void stop();
    void runner();

    void setup();
    void write();
    void read();
    void shutdown();

  private:
    const std::string getErrorStr( CassFuture*& future );
    void readIntoBinary( daq::utilities::Binary& binary, const CassValue* const & value );
    bool prepareQuery( const std::string& qStr, const CassPrepared** prepared );
    bool executeStatement( CassStatement*& statement );
    bool executeStatement( CassStatement*& statement, const CassResult** result );
    bool executeQuery( const std::string& queryStr );
    bool columnFamilyExists( const std::string& cfName ); 

    CassandraChunkedStorageProvider m_chunkProvider;

    CassCluster* m_cluster;
    CassSession* m_session;

};

#endif /* DAQ_MODULES_CASSANDRADATALOGGER_HPP_ */

