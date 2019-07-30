file(GLOB_RECURSE BOARDREADERBINARY_SOURCES "BoardReaderBinary.cpp")
add_library(boardreaderbinary SHARED ${BOARDREADERBINARY_SOURCES})
target_link_libraries(boardreaderbinary PUBLIC DaqlingCore DaqlingUtils libzmq)
