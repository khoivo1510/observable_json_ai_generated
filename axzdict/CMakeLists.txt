cmake_minimum_required( VERSION 3.5.0 )

project( axzdct )

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ( MSVC )
    add_definitions( -DAXZDICT_EXPORT -DUNICODE )
endif()

set( CMAKE_SKIP_BUILD_RPATH FALSE )
set( CMAKE_BUILD_WITH_INSTALL_RPATH TRUE )
set( CMAKE_INSTALL_RPATH "$ORIGIN/" )

# for module sources
file( GLOB LIB_DICT_SOURCES *.cpp )

# do generate shared library from sources
add_library( ${PROJECT_NAME} SHARED ${LIB_DICT_SOURCES} )


if ( UNIX AND NOT APPLE )
    target_link_libraries( ${PROJECT_NAME} gcc_s pthread )
    if ( UNIX AND NOT APPLE )
        include(dbgGenerator OPTIONAL RESULT_VARIABLE _module_path)
        if (NOT _module_path)
            message("Failed to find module dbgGenerator")
        endif()
    endif()
endif()

# for version (define AXZ_AGENT_VERSION_STRING if not defined)
if(NOT DEFINED AXZ_AGENT_VERSION_STRING)
    set(AXZ_AGENT_VERSION_STRING "2.0.0")
endif()

set_target_properties( ${PROJECT_NAME} PROPERTIES VERSION ${AXZ_AGENT_VERSION_STRING} )
