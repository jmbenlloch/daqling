add_executable(test_plugin src/test_plugin.cpp)
target_link_libraries(test_plugin PUBLIC ${DAQLING_DEPS_LIBRARIES} ${DAQLING_LIBRARIES})
install(TARGETS test_plugin EXPORT DESTINATION bin)

add_executable(test_logging src/test_logging.cpp)
target_link_libraries(test_logging PUBLIC ${DAQLING_DEPS_LIBRARIES} ${DAQLING_LIBRARIES})
install(TARGETS test_logging EXPORT DESTINATION bin)

add_executable(test_json src/test_json.cpp)
target_link_libraries(test_json PUBLIC ${DAQLING_DEPS_LIBRARIES} ${DAQLING_LIBRARIES})
install(TARGETS test_json EXPORT DESTINATION bin)

add_executable(test_metrics src/test_metrics.cpp)
target_link_libraries(test_metrics PUBLIC ${DAQLING_DEPS_LIBRARIES} ${DAQLING_LIBRARIES})
install(TARGETS test_metrics EXPORT DESTINATION bin)

if (ENABLE_TBB)
  add_executable(test_flowgraph src/test_flowgraph.cpp)
  target_link_libraries(test_flowgraph PUBLIC ${DAQLING_DEPS_LIBRARIES} ${DAQLING_LIBRARIES})
  install(TARGETS test_flowgraph EXPORT DESTINATION bin)
endif (ENABLE_TBB)

add_executable(test_pub_topic src/test_pub_topic.cpp)
target_link_libraries(test_pub_topic PUBLIC ${DAQLING_DEPS_LIBRARIES} ${DAQLING_LIBRARIES})
install(TARGETS test_pub_topic EXPORT DESTINATION bin)

add_executable(test_sub_topic src/test_sub_topic.cpp)
target_link_libraries(test_sub_topic PUBLIC ${DAQLING_DEPS_LIBRARIES} ${DAQLING_LIBRARIES})
install(TARGETS test_sub_topic EXPORT DESTINATION bin)
