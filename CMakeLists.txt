cmake_minimum_required(VERSION 3.16)
project(UniversalObservableJson 
    VERSION 1.0.0
    DESCRIPTION "Universal Observable JSON library with multi-backend support"
    LANGUAGES CXX
)

# ==================== CONFIGURATION ====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Modern CMake best practices
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra -Wpedantic)
endif()

# ==================== BACKEND SELECTION ====================
option(USE_JSON11 "Use json11 backend instead of nlohmann/json" OFF)
option(USE_RAPIDJSON "Use RapidJSON backend instead of nlohmann/json" OFF)

if(USE_RAPIDJSON)
    message(STATUS "Using RapidJSON backend")
    set(JSON_BACKEND_MACRO "JSON_ADAPTER_BACKEND=3")
elseif(USE_JSON11)
    message(STATUS "Using json11 backend")
    set(JSON_BACKEND_MACRO "JSON_ADAPTER_BACKEND=2")
else()
    message(STATUS "Using nlohmann/json backend (default)")
    set(JSON_BACKEND_MACRO "JSON_ADAPTER_BACKEND=1")
endif()

# ==================== DEPENDENCIES ====================
include(FetchContent)

if(USE_RAPIDJSON)
    # Fetch RapidJSON
    FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_TAG v1.1.0
    )
    FetchContent_MakeAvailable(rapidjson)
elseif(USE_JSON11)
    # Fetch json11
    FetchContent_Declare(
        json11
        GIT_REPOSITORY https://github.com/dropbox/json11.git
        GIT_TAG v1.0.0
    )
    FetchContent_MakeAvailable(json11)
else()
    # Fetch nlohmann/json
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Threading support
find_package(Threads REQUIRED)

# ==================== LIBRARY ====================
# Header-only library
add_library(universal_observable_json INTERFACE)
target_include_directories(universal_observable_json INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Set backend macro
target_compile_definitions(universal_observable_json INTERFACE ${JSON_BACKEND_MACRO})

# Link appropriate JSON library
if(USE_RAPIDJSON)
    target_include_directories(universal_observable_json INTERFACE
        ${rapidjson_SOURCE_DIR}/include
    )
    target_link_libraries(universal_observable_json INTERFACE 
        Threads::Threads
    )
elseif(USE_JSON11)
    target_link_libraries(universal_observable_json INTERFACE 
        json11
        Threads::Threads
    )
else()
    target_link_libraries(universal_observable_json INTERFACE 
        nlohmann_json::nlohmann_json
        Threads::Threads
    )
endif()

# ==================== TESTS ====================
option(BUILD_TESTS "Build test suite" ON)

if(BUILD_TESTS)
    add_executable(comprehensive_test tests/comprehensive_test.cpp)
    target_link_libraries(comprehensive_test PRIVATE universal_observable_json)
    
    # Test runner
    enable_testing()
    add_test(NAME comprehensive_test COMMAND comprehensive_test)
endif()

# ==================== EXAMPLES ====================
option(BUILD_EXAMPLES "Build example programs" ON)

if(BUILD_EXAMPLES)
    add_executable(basic_example examples/basic_example.cpp)
    target_link_libraries(basic_example PRIVATE universal_observable_json)
    
    add_executable(performance_comparison examples/performance_comparison.cpp)
    target_link_libraries(performance_comparison PRIVATE universal_observable_json)
    
    add_executable(multi_backend_demo examples/multi_backend_demo.cpp)
    target_link_libraries(multi_backend_demo PRIVATE universal_observable_json)
endif()

# ==================== INSTALLATION ====================
include(GNUInstallDirs)

# Install headers
install(DIRECTORY include/ 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# ==================== SUMMARY ====================
message(STATUS "Universal Observable JSON Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Backend: ${JSON_BACKEND_MACRO}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
if(USE_RAPIDJSON)
    message(STATUS "  JSON Backend: RapidJSON")
elseif(USE_JSON11)
    message(STATUS "  JSON Backend: json11")
else()
    message(STATUS "  JSON Backend: nlohmann/json (default)")
endif()