file(GLOB_RECURSE NEWMODULE_SOURCES "NewModule.cpp")
add_library(newmodule_daqling SHARED ${NEWMODULE_SOURCES})
target_link_libraries(newmodule_daqling PUBLIC DaqlingCore DaqlingUtils libzmq)
