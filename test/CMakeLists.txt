cmake_minimum_required(VERSION 3.4.3)

#library
project(test VERSION 0.0.1
        DESCRIPTION "Data acquisition framework"
        LANGUAGES CXX)

include_directories("../include")

add_executable(test_plugin src/test_plugin.cpp)
target_link_libraries(test_plugin PUBLIC pthread rt utilities boardreader eventbuilder)
install(TARGETS test_plugin EXPORT DESTINATION bin)

add_executable(test_logging src/test_logging.cpp)
target_link_libraries(test_logging PUBLIC pthread rt utilities core)
install(TARGETS test_logging EXPORT DESTINATION bin)

add_executable(test_json src/test_json.cpp)
target_link_libraries(test_json PUBLIC pthread rt utilities core)
install(TARGETS test_json EXPORT DESTINATION bin)

add_executable(test_metrics src/test_metrics.cpp)
target_link_libraries(test_metrics PUBLIC pthread rt utilities zmq)
install(TARGETS test_metrics EXPORT DESTINATION bin)

if (ENABLE_TBB)
  add_executable(test_flowgraph src/test_flowgraph.cpp)
  target_link_libraries(test_flowgraph PUBLIC pthread rt utilities tbb)
  install(TARGETS test_flowgraph EXPORT DESTINATION bin)
endif (ENABLE_TBB)

add_executable(test_pub_topic src/test_pub_topic.cpp)
target_link_libraries(test_pub_topic PUBLIC pthread rt utilities zmq)
install(TARGETS test_pub_topic EXPORT DESTINATION bin)

add_executable(test_sub_topic src/test_sub_topic.cpp)
target_link_libraries(test_sub_topic PUBLIC pthread rt utilities zmq)
install(TARGETS test_sub_topic EXPORT DESTINATION bin)
