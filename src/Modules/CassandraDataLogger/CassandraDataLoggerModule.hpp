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

#pragma once

/// \cond
#include <cassandra.h>
#include <iostream>
/// \endcond

#include "Core/DAQProcess.hpp"
#include "Core/DataLogger.hpp"
#include "Utils/Binary.hpp"
#include "Utils/ChunkedStorage.hpp"

//#define HASH_MODE
#define EVID_MODE

/*
 * CassandraDataLoggerModule
 * Description: Data logger with Cassandra persistency layer
 *   Heavily relies on the CondDB payload chunked storage.
 * Date: November 2017
 */
class CassandraDataLoggerModule : public daqling::core::DAQProcess,
                                  public daqling::core::DataLogger {
  class CassandraChunkedStorageProvider : public daqling::persistency::ChunkedStorageProvider {
  public:
    explicit CassandraChunkedStorageProvider() {
      // Any init?
    }

    virtual ~CassandraChunkedStorageProvider(){};
    bool exists() { return true; }
    void create(){};
    const size_t writeChunk(const std::string &objectName, int chunkId,
                            const std::pair<const void *, size_t> &data, int ttl) const {
      return 0;
    }
    bool readChunk(const std::string &objectName, int chunkId, size_t split, void *&blobPtr) const {
      return false;
    };
    const int getDefaultChunkSize() { return 0; }
    void deleteObject(const std::string &objectName, int chunkCount) const {}
    void writeMetadata(const std::string &objectName,
                       const daqling::persistency::ObjectMetadata &attr) const {}
    const daqling::persistency::ObjectMetadata readMetadata(const std::string &objectName) const {
      daqling::persistency::ObjectMetadata attributes;
      return attributes;
    }

  private:
    const std::string M_NAME = "chunk";
  };

public:
  CassandraDataLoggerModule();
  ~CassandraDataLoggerModule();

  void start();
  void stop();
  void runner();

  void setup();
  void write();
  void read();
  bool write(uint64_t keyId, daqling::utilities::Binary &payload);
  void shutdown();

private:
  // RS -> ALL THIS SHOULD BE NICELY HIDDEN BEHIND A SESSION LAYER.
  const std::string M_KEYSPACE_NAME = "daq";
  const std::string M_CF_NAME = "payload";

  const std::string M_COLUMNFAMILY = "pkey, type, s_info, version, time, size, data";
  const std::string M_COLUMN_KEY = "pkey";
  const std::string M_COLUMN_TYPE = "type";
  const std::string M_COLUMN_SINFO = "s_info";
  const std::string M_COLUMN_VERSION = "version";
  const std::string M_COLUMN_TIME = "time";
  const std::string M_COLUMN_SIZE = "size";
  const std::string M_COLUMN_DATA = "data";

  const std::string Q_SAY_HI =
      "SELECT key,bootstrapped,broadcast_address,cluster_name,cql_version,data_center FROM "
      "system.local";
  const std::string Q_CF_EXISTS =
      "SELECT table_name from system_schema.tables WHERE keyspace_name=? AND table_name=?";
  const std::string Q_INSERT = "INSERT INTO daq.payload (pkey, type, s_info, version, time, size, "
                               "data) VALUES (?,?,?,?,?,?,?);";

  const std::string getErrorStr(CassFuture *&future);
  void readIntoBinary(daqling::utilities::Binary &binary, const CassValue *const &value);
  bool prepareQuery(const std::string &qStr, const CassPrepared **prepared);
  bool executeStatement(CassStatement *&statement);
  bool executeStatement(CassStatement *&statement, const CassResult **result);
  bool executeQuery(const std::string &queryStr);
  bool columnFamilyExists(const std::string &cfName);

  bool exists();
  bool create();

  CassandraChunkedStorageProvider m_chunkProvider;

  CassCluster *m_cluster;
  CassSession *m_session;
};
