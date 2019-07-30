file(GLOB_RECURSE BOARDREADER_SOURCES "BoardReader.cpp")
add_library(boardreader SHARED ${BOARDREADER_SOURCES})
target_link_libraries(boardreader PUBLIC DaqlingCore DaqlingUtils libzmq)
