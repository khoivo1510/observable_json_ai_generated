#!/bin/bash

# Comprehensive Analysis - Universal Observable JSON
# Tests all 5 backends: nlohmann/json, json11, RapidJSON, JsonCpp, AxzDict

echo "Universal Observable JSON - Comprehensive Analysis"
echo "================================================="

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
declare -A test_results
declare -A performance_results
declare -A memory_results

# Function to analyze a backend
analyze_backend() {
    local backend_name=$1
    local cmake_opts=$2
    local build_dir="analysis_${backend_name}"
    
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}Analyzing ${backend_name} backend${NC}"
    echo -e "${BLUE}================================${NC}"
    
    # Clean and create build directory
    rm -rf "$build_dir"
    mkdir "$build_dir"
    cd "$build_dir"
    
    # Configure
    echo -e "${CYAN}Configuring ${backend_name}...${NC}"
    if cmake $cmake_opts -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" .. > configure.log 2>&1; then
        echo -e "${GREEN}✓ Configure successful${NC}"
    else
        echo -e "${RED}✗ Configure failed${NC}"
        test_results[$backend_name]="CONFIG_FAIL"
        performance_results[$backend_name]="CONFIG_FAIL"
        memory_results[$backend_name]="CONFIG_FAIL"
        cd ..
        return 1
    fi
    
    # Build
    echo -e "${CYAN}Building ${backend_name}...${NC}"
    if make performance_comparison comprehensive_test > build.log 2>&1; then
        echo -e "${GREEN}✓ Build successful${NC}"
    else
        echo -e "${RED}✗ Build failed${NC}"
        test_results[$backend_name]="BUILD_FAIL"
        performance_results[$backend_name]="BUILD_FAIL"
        memory_results[$backend_name]="BUILD_FAIL"
        cd ..
        return 1
    fi
    
    # Find executables
    perf_exe=$(find . -name "performance_comparison" -type f | head -1)
    test_exe=$(find . -name "comprehensive_test" -type f | head -1)
    
    if [ -z "$perf_exe" ] || [ -z "$test_exe" ]; then
        echo -e "${RED}✗ Executables not found${NC}"
        test_results[$backend_name]="EXE_NOT_FOUND"
        performance_results[$backend_name]="EXE_NOT_FOUND"
        memory_results[$backend_name]="EXE_NOT_FOUND"
        cd ..
        return 1
    fi
    
    # Run comprehensive test
    echo -e "${CYAN}Running comprehensive tests...${NC}"
    if timeout 300 "$test_exe" > test_output.log 2>&1; then
        if grep -q "ALL TESTS PASSED!" test_output.log; then
            echo -e "${GREEN}✓ All tests passed${NC}"
            test_results[$backend_name]="PASS"
        else
            echo -e "${RED}✗ Some tests failed${NC}"
            test_results[$backend_name]="FAIL"
        fi
    else
        echo -e "${RED}✗ Tests timeout or crashed${NC}"
        test_results[$backend_name]="TIMEOUT"
    fi
    
    # Run performance test
    echo -e "${CYAN}Running performance test...${NC}"
    if timeout 300 "$perf_exe" > perf_output.log 2>&1; then
        # Extract timing data
        TOTAL_TIME=$(grep "Total benchmark time:" perf_output.log | grep -o "[0-9]\+ ms" | head -1 || echo "N/A")
        if [ "$TOTAL_TIME" != "N/A" ]; then
            echo -e "${GREEN}✓ Performance test completed: $TOTAL_TIME${NC}"
            performance_results[$backend_name]="$TOTAL_TIME"
        else
            performance_results[$backend_name]="Performance test completed - no timing data"
        fi
    else
        echo -e "${RED}✗ Performance test failed${NC}"
        performance_results[$backend_name]="PERF_FAIL"
    fi
    
    # Memory analysis
    echo -e "${CYAN}Running memory analysis...${NC}"
    if command -v valgrind > /dev/null 2>&1; then
        valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
                 --error-exitcode=1 --log-file=valgrind.log \
                 "$test_exe" > /dev/null 2>&1
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ No memory leaks detected${NC}"
            memory_results[$backend_name]="PASS - No leaks"
        else
            # Extract memory info
            definitely_lost=$(grep "definitely lost:" valgrind.log | grep -o "[0-9,]\+ bytes" | head -1 || echo "0 bytes")
            echo -e "${YELLOW}⚠️ Memory issues detected: $definitely_lost lost${NC}"
            memory_results[$backend_name]="ISSUES - $definitely_lost lost"
        fi
    else
        memory_results[$backend_name]="Valgrind not available"
    fi
    
    cd ..
    echo ""
}

# Test all backends
echo -e "${YELLOW}Starting comprehensive analysis...${NC}"
echo ""

for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    analyze_backend "$backend" "${backends[$backend]}"
done

# Print results summary
echo -e "${PURPLE}========================================${NC}"
echo -e "${PURPLE}COMPREHENSIVE ANALYSIS SUMMARY${NC}"
echo -e "${PURPLE}========================================${NC}"
echo ""

echo -e "${BLUE}Test Results:${NC}"
echo "-------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${test_results[$backend]:-"NOT_TESTED"}
    if [ "$result" = "PASS" ]; then
        echo -e "  ${backend}: ${GREEN}✓ $result${NC}"
    else
        echo -e "  ${backend}: ${RED}✗ $result${NC}"
    fi
done

echo ""
echo -e "${BLUE}Performance Results:${NC}"
echo "--------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${performance_results[$backend]:-"NOT_TESTED"}
    if [[ "$result" != *"FAIL"* ]] && [[ "$result" != "NOT_TESTED" ]]; then
        echo -e "  ${backend}: ${GREEN}$result${NC}"
    else
        echo -e "  ${backend}: ${RED}$result${NC}"
    fi
done

echo ""
echo -e "${BLUE}Memory Analysis Results:${NC}"
echo "------------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${memory_results[$backend]:-"NOT_TESTED"}
    if [[ "$result" == "PASS"* ]]; then
        echo -e "  ${backend}: ${GREEN}✓ $result${NC}"
    elif [[ "$result" == "ISSUES"* ]]; then
        echo -e "  ${backend}: ${YELLOW}⚠️ $result${NC}"
    else
        echo -e "  ${backend}: ${RED}$result${NC}"
    fi
done

echo ""
echo -e "${BLUE}Analysis completed for all backends:${NC}"
echo "  - nlohmann/json: Full-featured JSON library"
echo "  - json11: Lightweight JSON library"  
echo "  - RapidJSON: Fast JSON parser/generator"
echo "  - JsonCpp: Mature JSON library"
echo "  - AxzDict: Advanced dictionary-based JSON implementation"

echo ""
echo -e "${GREEN}All analysis logs have been saved in respective directories.${NC}"

# Cleanup option
echo ""
read -p "Clean up temporary directories? (y/n): " -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf analysis_*
    echo "Cleanup completed."
else
    echo "Temporary directories preserved for manual inspection."
fi

echo ""
echo -e "${PURPLE}Comprehensive analysis completed successfully!${NC}"
