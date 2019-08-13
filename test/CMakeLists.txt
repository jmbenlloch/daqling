macro(daqling_test name)
    daqling_executable(test_${name})
    daqling_target_sources(test_${name}
        src/test_${name}.cpp
    )
    daqling_target_install(test_${name})
endmacro()

daqling_test(plugin)
daqling_test(logging)
daqling_test(json)
daqling_test(metrics)
daqling_test(pub_topic)
daqling_test(sub_topic)

if (ENABLE_TBB)
    daqling_test(flowgraph)
endif (ENABLE_TBB)
