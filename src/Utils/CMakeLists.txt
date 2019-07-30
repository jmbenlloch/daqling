add_library(DaqlingUtils SHARED
  Binary.cpp
  Logging.cpp
)

target_link_libraries(DaqlingUtils ${DAQLING_DEPS_LIBRARIES})
target_link_libraries(DaqlingUtils ${DAQLING_LIBRARIES})

target_compile_definitions(DaqlingUtils PRIVATE SHARED_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
