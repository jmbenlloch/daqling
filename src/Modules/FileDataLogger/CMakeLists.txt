file(GLOB_RECURSE FILEDATALOGGER_SOURCES "FileDataLogger.cpp")
add_library(filedatalogger SHARED ${FILEDATALOGGER_SOURCES})
target_link_libraries(filedatalogger PUBLIC DaqlingCore DaqlingUtils libzmq)
