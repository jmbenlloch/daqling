# Include the dependencies
include_directories(SYSTEM ${DAQLING_DEPS_INCLUDE_DIRS})

# Create Utils library
add_library(DaqlingUtils SHARED
  Binary.cpp
  Logging.cpp
)

# Link the dependencies
target_link_libraries(DaqlingUtils ${DAQLING_DEPS_LIBRARIES})
target_link_libraries(DaqlingUtils ${DAQLING_LIBRARIES})

# Define compile-time library extension
target_compile_definitions(DaqlingUtils PRIVATE SHARED_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")

# Create standard install target
install(TARGETS DaqlingUtils
    COMPONENT application
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)
