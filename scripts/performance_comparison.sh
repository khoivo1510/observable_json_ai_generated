#!/bin/bash

# Performance Comparison Script
# Compares performance of nlohmann/json, json11, and RapidJSON backends

echo "Universal Observable JSON - Performance Comparison"
echo "=================================================="

# Function to run performance test
run_performance_test() {
    local backend=$1
    local cmake_flags=$2
    local build_dir="perf_${backend//\//_}"  # Replace / with _ for directory name
    
    echo "Testing $backend backend..."
    
    # Clean and create build directory
    rm -rf $build_dir
    mkdir $build_dir
    cd $build_dir
    
    # Configure and build
    cmake $cmake_flags -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1
    make performance_comparison > /dev/null 2>&1
    
    # Run performance test
    echo "Results for $backend:"
    ./performance_comparison
    echo ""
    
    cd ..
}

# Test all backends
run_performance_test "nlohmann/json" "-DUSE_JSON11=OFF -DUSE_RAPIDJSON=OFF"
run_performance_test "json11" "-DUSE_JSON11=ON"
run_performance_test "RapidJSON" "-DUSE_RAPIDJSON=ON"

echo "Performance comparison completed."
echo "Tested backends: nlohmann/json, json11, RapidJSON"

# Cleanup
rm -rf perf_*
