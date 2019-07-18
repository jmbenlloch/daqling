cmake_minimum_required(VERSION 3.4.3)

set(PACKAGE daqling)
set(PACKAGE_VERSION 0.0.1)

project(${PACKAGE} VERSION ${PACKAGE_VERSION}
        DESCRIPTION "Data acquisition framework"
        LANGUAGES CXX)

find_package(Doxygen)
if (DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started")
  add_custom_target( doc_doxygen 
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    message(FATAL_ERROR "GCC version must be at least 4.8!")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopt-prefetch -unroll-aggressive -march=native -mtune=native")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wno-unused-function -Wno-unused-variable -O3 -g -pedantic -fPIC")

set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -ldl")

#message("CMAKE_CXX_FLAGS = " ${CMAKE_CXX_FLAGS})
#message("CMAKE_CXX_LINK_FLAGS = " ${CMAKE_CXX_LINK_FLAGS})

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

# others
include_directories(/opt/spdlog/include)

include_directories(/opt/json/single_include)

include_directories(/opt/cpr/include)
link_directories(/opt/cpr/build/lib)

# pass include directories to parent
get_directory_property(hasParent PARENT_DIRECTORY)
if (hasParent)
  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
  set(daqling_include_dirs ${dirs} PARENT_SCOPE)
endif (hasParent)

# pass link directories to parent
if (hasParent)
  get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY LINK_DIRECTORIES)
  set(daqling_link_dirs ${dirs} PARENT_SCOPE)
endif (hasParent)

add_subdirectory(src/Utilities)
add_subdirectory(src/Core)
add_subdirectory(src/Modules)
add_subdirectory(test)
