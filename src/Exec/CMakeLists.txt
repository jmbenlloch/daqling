add_executable(daqling daqling.cpp)
target_link_libraries(daqling ${DAQLING_DEPS_LIBRARIES} rt DaqlingUtils DaqlingCore)
install(TARGETS daqling EXPORT DESTINATION bin)
