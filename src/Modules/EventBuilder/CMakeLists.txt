file(GLOB_RECURSE EVENTBUILDER_SOURCES "EventBuilder.cpp")
add_library(eventbuilder SHARED ${EVENTBUILDER_SOURCES})
target_link_libraries(eventbuilder PUBLIC DaqlingCore DaqlingUtils libzmq)
