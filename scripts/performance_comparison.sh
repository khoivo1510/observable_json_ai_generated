#!/bin/bash

# Performance Comparison Script
# Compares performance of nlohmann/json, json11, RapidJSON, JsonCpp, and AxzDict backends

echo "Universal Observable JSON - Performance Comparison"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to run performance test
run_performance_test() {
    local backend=$1
    local cmake_flags=$2
    local build_dir="perf_${backend//\//_}"  # Replace / with _ for directory name
    
    echo -e "${BLUE}Testing $backend backend...${NC}"
    
    # Clean and create build directory
    rm -rf $build_dir
    mkdir $build_dir
    cd $build_dir
    
    # Configure and build
    if cmake $cmake_flags -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1; then
        if make performance_comparison > /dev/null 2>&1; then
            echo -e "${GREEN}✓ Build successful${NC}"
            
            # Run performance test
            echo "Results for $backend:"
            # Check if executable is in bin/ directory
            if [ -f ./bin/performance_comparison ]; then
                ./bin/performance_comparison
            elif [ -f ./performance_comparison ]; then
                ./performance_comparison
            else
                echo -e "${RED}✗ Performance test executable not found${NC}"
            fi
            echo ""
        else
            echo -e "${RED}✗ Build failed${NC}"
        fi
    else
        echo -e "${RED}✗ Configure failed${NC}"
    fi
    
    cd ..
}

# Test all backends
run_performance_test "nlohmann/json" ""
run_performance_test "json11" "-DUSE_JSON11=ON"
run_performance_test "RapidJSON" "-DUSE_RAPIDJSON=ON"
run_performance_test "JsonCpp" "-DUSE_JSONCPP=ON"
run_performance_test "AxzDict" "-DUSE_AXZDICT=ON"

echo "Performance comparison completed."
echo "Tested backends: nlohmann/json, json11, RapidJSON, JsonCpp, AxzDict"

# Cleanup
rm -rf perf_*
