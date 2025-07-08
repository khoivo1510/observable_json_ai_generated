cmake_minimum_required(VERSION 3.16)
project(ObservableJsonUltimatePro 
    VERSION 1.0.0
    DESCRIPTION "The definitive C++17 reactive JSON library with Thread Pool and modern async patterns"
    LANGUAGES CXX
)

# ==================== CONFIGURATION ====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Modern CMake best practices
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra -Wpedantic)
endif()

# ==================== DEPENDENCIES ====================
include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# Find threads package
find_package(Threads REQUIRED)

# ==================== OBSERVABLE JSON LIBRARY ====================
add_library(observable_json INTERFACE)

# Modern target configuration
target_include_directories(observable_json INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(observable_json INTERFACE 
    nlohmann_json::nlohmann_json
    Threads::Threads
)

target_compile_features(observable_json INTERFACE cxx_std_17)

# ==================== ULTIMATE DEMO EXECUTABLE ====================
add_executable(ultimate_demo source/main.cpp)

target_link_libraries(ultimate_demo PRIVATE 
    observable_json
    Threads::Threads
)

# Enable all warnings for demo
target_compile_options(ultimate_demo PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# ==================== INSTALLATION (SIMPLIFIED) ====================
include(GNUInstallDirs)

# Install header files
install(DIRECTORY include/ 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# ==================== SUMMARY ====================
message(STATUS "Observable JSON Ultimate Pro Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Hardware threads: ${CMAKE_THREAD_PREFER_PTHREAD}")