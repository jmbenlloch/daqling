add_library(DaqlingCore SHARED
  Core.cpp
  Command.cpp
  ConnectionManager.cpp
  PluginManager.cpp
  Statistics.cpp
)

find_package(CURL QUIET REQUIRED)

# target_link_libraries(DaqlingCore ${DAQLING_DEPS_LIBRARIES})
# target_link_libraries(DaqlingCore ${DAQLING_LIBRARIES})

target_link_libraries(DaqlingCore PUBLIC DaqlingUtils libzmq cpr ${CURL_LIBRARIES} dl)

#executable
# add_executable(main_core main_core.cpp)
# target_link_libraries(main_core PUBLIC pthread rt utilities core)
# install(TARGETS main_core EXPORT DESTINATION bin)
