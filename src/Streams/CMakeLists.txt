add_library(DaqlingStreams SHARED DaqlingStandardStream.cpp ZMQSinkStream.cpp)
set(ENV{TDAQ_ERS_STREAM_LIBS} DaqlingStandardStream)
set(ENV{TDAQ_ERS_STREAM_LIBS} ZMQSinkStream)

