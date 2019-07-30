file(GLOB_RECURSE EVENTBUILDERBINARY_SOURCES "EventBuilderBinary.cpp")
add_library(eventbuilderbinary SHARED ${EVENTBUILDERBINARY_SOURCES})
target_link_libraries(eventbuilderbinary PUBLIC DaqlingCore DaqlingUtils libzmq)
