# Adapted from <https://gitlab.cern.ch/allpix-squared/allpix-squared/blob/276d03f811de400c704127ee341dbd1566225408/cmake/allpix.cmake>

macro(daqling_module name)
    # Get the name of the module
    get_filename_component(_daqling_module_dir ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    # Build all modules by default if not specified otherwise
    option(BUILD_${_daqling_module_dir} "Build module in directory ${_daqling_module_dir}?" ON)
    message(STATUS "Building module ${_daqling_module_dir} \t - ${BUILD_${_daqling_module_dir}}")

    # Quit the file if not building this file or all modules
    if(NOT (BUILD_${_daqling_module_dir} OR BUILD_ALL_MODULES))
        return()
    endif()

    # Prepend with the Daqling module prefix to create the name of the module
    set(${name} "DaqlingModule${_daqling_module_dir}")

    # Set module class name
    set(_daqling_module_class "${_daqling_module_dir}Module")

    # Check if main header file is defined
    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_daqling_module_class}.hpp")
        message(FATAL_ERROR "Header file ${_daqling_module_class}.hpp does not exist, cannot build module!")
    endif()

    # Define the library
    add_library(${${name}} SHARED "")

    # Add the current directory as include directory
    target_include_directories(${${name}} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # Set the special header flags and add the special dynamic implementation file
    target_compile_definitions(${${name}} PRIVATE DAQLING_MODULE_NAME=${_daqling_module_class})
    target_compile_definitions(${${name}} PRIVATE DAQLING_MODULE_HEADER="${_daqling_module_class}.hpp")

    target_sources(${${name}} PRIVATE "${PROJECT_SOURCE_DIR}/src/Core/dynamic_module_impl.cpp")
    set_property(SOURCE "${PROJECT_SOURCE_DIR}/src/Core/dynamic_module_impl.cpp" APPEND PROPERTY OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_daqling_module_class}.hpp")
endmacro()

# Add sources to the module
macro(daqling_module_sources name)
    # Get the list of sources
    set(_list_var "${ARGN}")
    list(REMOVE_ITEM _list_var ${name})

    # Include directories for dependencies
    # include_directories(SYSTEM ${ALLPIX_DEPS_INCLUDE_DIRS})

    # Add the library
    target_sources(${name} PRIVATE ${_list_var})

    # Link the standard allpix libraries
    # target_link_libraries(${name} ${ALLPIX_LIBRARIES} ${ALLPIX_DEPS_LIBRARIES})
    # target_link_libraries(${name} DaqlingCore DaqlingUtils libzmq)
endmacro()

# Provide default install target for the module
macro(daqling_module_install name)
    install(TARGETS ${name}
        COMPONENT modules
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
endmacro()
