# Include the dependencies
include_directories(SYSTEM ${DAQLING_DEPS_INCLUDE_DIRS})

# Create core library
add_library(DaqlingCore SHARED
  Core.cpp
  Command.cpp
  ConnectionManager.cpp
  PluginManager.cpp
  Statistics.cpp
)

# Link the dependencies
target_link_libraries(DaqlingCore ${DAQLING_DEPS_LIBRARIES})
target_link_libraries(DaqlingCore ${DAQLING_LIBRARIES})

# Define compile-time library extension
target_compile_definitions(DaqlingCore PRIVATE SHARED_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
# Link the DL libraries
target_link_libraries(DaqlingCore ${CMAKE_DL_LIBS})

# Create install target
install(TARGETS DaqlingCore
    COMPONENT application
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
