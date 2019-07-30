file(GLOB_RECURSE METRICSSIMULATOR_SOURCES "MetricsSimulator.cpp")
add_library(metricssimulator SHARED ${METRICSSIMULATOR_SOURCES})
target_link_libraries(metricssimulator PUBLIC DaqlingCore DaqlingUtils libzmq)

