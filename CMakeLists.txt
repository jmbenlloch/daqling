cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)
cmake_policy(SET CMP0077 NEW)
set(PACKAGE daqling)
set(PACKAGE_VERSION 0.0.1)

project(${PACKAGE} VERSION ${PACKAGE_VERSION}
        DESCRIPTION "Data acquisition framework"
        LANGUAGES C CXX)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build. Options are: Debug RelWithDebInfo" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Print version
message(STATUS "Building ${PACKAGE} version ${PACKAGE_VERSION}")
message(STATUS "Building \"${CMAKE_BUILD_TYPE}\"")

###############################################
# Setup the environment for the documentation #
###############################################

option(BUILD_DOCS_ONLY "Build documentation only" OFF)

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
    )
else(DOXYGEN_FOUND)
  message("Doxygen not found. Cannot build documentation.")
endif(DOXYGEN_FOUND)

# If only building docs, stop processing the rest of the CMake file:
if(BUILD_DOCS_ONLY)
  RETURN()
ENDIF()

###############################
# Setup the build environment #
###############################

include(cmake/daqling.cmake)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
  message(FATAL_ERROR "GCC version must be at least 4.8!")
endif()

# Require a C++17 compliant compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (ENABLE_TBB)
  set(tbb_version $ENV{TBB_VERSION})
  set(TBB_ROOT_DIR $ENV{TBB_ROOT_DIR})
  find_package(TBB ${tbb_version} EXACT REQUIRED)
  include_directories(${TBB_INCLUDE_DIRS})
  link_directories(${TBB_ROOT_DIR}/lib)
endif (ENABLE_TBB)

if (ENABLE_BOOST)
  set(boost_version $ENV{BOOST_VERSION})
  set(BOOST_ROOT $ENV{BOOST_ROOT_DIR})
  find_package(Boost ${boost_version} EXACT REQUIRED COMPONENTS system filesystem regex)
  include_directories(${Boost_INCLUDE_DIRS})
endif (ENABLE_BOOST)

set(BUILD_TESTS OFF) # disable zmq tests
set(USE_SYSTEM_CURL ON)
set(BUILD_CPR_TESTS OFF)
set(ZMQ_BUILD_TESTS OFF)

set(BUILD_SHARED_LIBS ON)

########################
# Define build targets #
########################

set(DAQLING_LIBRARIES "")

add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/zmq)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/cpr)

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/3rdparty/spdlog/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/3rdparty/json/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/3rdparty/cpr/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/3rdparty/zmq/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/3rdparty/cppzmq)

find_package(CURL QUIET REQUIRED)

set(DAQLING_DEPS_LIBRARIES libzmq Threads::Threads cpr)

# Set standard build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wzero-as-null-pointer-constant -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Wredundant-decls -Wsign-conversion -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Werror -Wformat-security -fdiagnostics-color=auto -Wno-overloaded-virtual")

# Disable some for convenience
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")

# Set some debug flags
if(CMAKE_BUILD_TYPE MATCHES Debug AND ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")))
    message(STATUS "Running debug build, adding extra sanitizer flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined")
endif()

# Set some release flags
if(CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "Running release build, adding optimization flags")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopt-prefetch -unroll-aggressive -march=native -mtune=native")
  endif()
endif()

set(BUILD_Dummy OFF)

include_directories(${CMAKE_SOURCE_DIR}/src)

# Build utils library
add_subdirectory(src/Utils)
set(DAQLING_LIBRARIES ${DAQLING_LIBRARIES} DaqlingUtils)

# Build core daqling library
add_subdirectory(src/Core)
set(DAQLING_LIBRARIES ${DAQLING_LIBRARIES} DaqlingCore)

# Build modules
add_subdirectory(src/Modules)

# Build the executable
add_subdirectory(src/Exec)

# Build tests
add_subdirectory(test)
