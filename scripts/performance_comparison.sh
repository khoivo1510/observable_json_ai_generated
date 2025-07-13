#!/bin/bash

# Performance Comparison Script - Universal Observable JSON
# Compares performance of nlohmann/json, json11, RapidJSON, JsonCpp, and AxzDict backends

echo "Universal Observable JSON - Performance Comparison"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Backend configurations
declare -A backends=(
    ["nlohmann_json"]=""
    ["json11"]="-DUSE_JSON11=ON"
    ["rapidjson"]="-DUSE_RAPIDJSON=ON"
    ["jsoncpp"]="-DUSE_JSONCPP=ON"
    ["axzdict"]="-DUSE_AXZDICT=ON"
)

# Results storage
declare -A performance_results

# Function to run performance test
run_performance_test() {
    local backend_name=$1
    local cmake_opts=$2
    local build_dir="perf_${backend_name}"
    
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}Testing ${backend_name} backend${NC}"
    echo -e "${BLUE}================================${NC}"
    
    # Clean and create build directory
    rm -rf "$build_dir"
    mkdir "$build_dir"
    cd "$build_dir"
    
    # Configure
    echo -e "${CYAN}Configuring ${backend_name}...${NC}"
    if cmake $cmake_opts -DCMAKE_BUILD_TYPE=Release .. > configure.log 2>&1; then
        echo -e "${GREEN}✓ Configure successful${NC}"
    else
        echo -e "${RED}✗ Configure failed${NC}"
        performance_results[$backend_name]="CONFIG_FAIL"
        cd ..
        return 1
    fi
    
    # Build
    echo -e "${CYAN}Building ${backend_name}...${NC}"
    if make performance_comparison > build.log 2>&1; then
        echo -e "${GREEN}✓ Build successful${NC}"
    else
        echo -e "${RED}✗ Build failed${NC}"
        performance_results[$backend_name]="BUILD_FAIL"
        cd ..
        return 1
    fi
    
    # Find executable
    perf_exe=$(find . -name "performance_comparison" -type f | head -1)
    if [ -z "$perf_exe" ]; then
        echo -e "${RED}✗ Executable not found${NC}"
        performance_results[$backend_name]="EXE_NOT_FOUND"
        cd ..
        return 1
    fi
    
    # Run performance test
    echo -e "${CYAN}Running performance test...${NC}"
    if timeout 300 "$perf_exe" > perf_output.log 2>&1; then
        # Show key results
        cat perf_output.log
        
        # Extract total benchmark time
        TOTAL_TIME=$(grep "Total benchmark time:" perf_output.log | grep -o "[0-9]\+ ms" | head -1 || echo "N/A")
        if [ "$TOTAL_TIME" != "N/A" ]; then
            echo -e "${GREEN}✓ Performance test completed: $TOTAL_TIME${NC}"
            performance_results[$backend_name]="$TOTAL_TIME"
        else
            performance_results[$backend_name]="Performance test completed - no timing data"
        fi
    else
        echo -e "${RED}✗ Performance test failed or timed out${NC}"
        performance_results[$backend_name]="PERF_FAIL"
    fi
    
    cd ..
    echo ""
}

# Main execution
echo -e "${YELLOW}Starting performance comparison...${NC}"
echo ""

# Test all backends
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    run_performance_test "$backend" "${backends[$backend]}"
done

# Print results summary
echo -e "${PURPLE}========================================${NC}"
echo -e "${PURPLE}PERFORMANCE COMPARISON SUMMARY${NC}"
echo -e "${PURPLE}========================================${NC}"
echo ""

echo -e "${BLUE}Performance Results (Total Benchmark Time):${NC}"
echo "--------------------------------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${performance_results[$backend]:-"NOT_TESTED"}
    if [[ "$result" != *"FAIL"* ]] && [[ "$result" != "NOT_TESTED" ]]; then
        echo -e "  ${backend}: ${GREEN}$result${NC}"
    else
        echo -e "  ${backend}: ${RED}$result${NC}"
    fi
done

echo ""
echo -e "${BLUE}Performance comparison completed for all backends:${NC}"
echo "  - nlohmann/json: Full-featured JSON library"
echo "  - json11: Lightweight JSON library"
echo "  - RapidJSON: Fast JSON parser/generator"
echo "  - JsonCpp: Mature JSON library"
echo "  - AxzDict: Advanced dictionary-based JSON implementation"

echo ""
echo -e "${GREEN}All performance logs have been saved in respective directories.${NC}"

# Cleanup option
echo ""
read -p "Clean up temporary directories? (y/n): " -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf perf_*
    echo "Cleanup completed."
else
    echo "Temporary directories preserved for manual inspection."
fi

echo ""
echo -e "${PURPLE}Performance comparison completed successfully!${NC}"