# Define module
daqling_module(module_name)

# Add source file to library
daqling_module_sources(${module_name}
    DummyModule.cpp
)

target_link_libraries(${module_name} DaqlingCore DaqlingUtils libzmq)

# Provide standard install target
daqling_module_install(${module_name})
