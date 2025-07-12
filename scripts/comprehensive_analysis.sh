#!/bin/bash

# Comprehensive Analysis - Universal Observable JSON
# Tests all 4 backends: nlohmann/json, json11, RapidJSON, JsonCpp

echo "Universal Observable JSON - Comprehensive Analysis"
echo "================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Backend configurations
declare -A backends=(
    ["nlohmann_json"]=""
    ["json11"]="-DUSE_JSON11=ON"
    ["rapidjson"]="-DUSE_RAPIDJSON=ON"
    ["jsoncpp"]="-DUSE_JSONCPP=ON"
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
    
    echo -e "${BLUE}Testing ${backend_name} backend...${NC}"
    echo "----------------------------------------"
    
    # Clean and create build directory
    rm -rf "$build_dir"
    mkdir "$build_dir"
    cd "$build_dir"
    
    # Configure
    echo "Configuring..."
    if cmake $cmake_opts -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" .. 2>&1; then
        echo -e "${GREEN}✓ Configure successful${NC}"
    else
        echo -e "${RED}✗ Configure failed${NC}"
        test_results[$backend_name]="CONFIG_FAIL"
        performance_results[$backend_name]="N/A"
        memory_results[$backend_name]="N/A"
        cd ..
        return 1
    fi
    
    # Build
    echo "Building..."
    if make performance_comparison comprehensive_test 2>&1; then
        echo -e "${GREEN}✓ Build successful${NC}"
    else
        echo -e "${RED}✗ Build failed${NC}"
        test_results[$backend_name]="BUILD_FAIL"
        performance_results[$backend_name]="N/A"
        memory_results[$backend_name]="N/A"
        cd ..
        return 1
    fi
    
    # Find executables
    perf_exe=$(find . -name "performance_comparison" -type f | head -1)
    test_exe=$(find . -name "comprehensive_test" -type f | head -1)
    
    if [ -z "$perf_exe" ] || [ -z "$test_exe" ]; then
        echo -e "${RED}✗ Executables not found${NC}"
        test_results[$backend_name]="EXE_NOT_FOUND"
        performance_results[$backend_name]="N/A"
        memory_results[$backend_name]="N/A"
        cd ..
        return 1
    fi
    
    # Run performance test
    echo "Running performance test..."
    if timeout 300 "$perf_exe" > perf_output.log 2>&1; then
        # Show actual output and extract performance data
        cat perf_output.log
        
        # Try multiple patterns to extract performance data
        perf_data=$(grep -E "ms|µs|ns|operations|time|duration|speed|benchmark" perf_output.log | head -5)
        
        if [ -n "$perf_data" ]; then
            performance_results[$backend_name]=$(echo "$perf_data" | tr '\n' '; ')
        else
            # If no specific patterns found, use the last few lines of output
            last_lines=$(tail -3 perf_output.log | grep -v "^$")
            if [ -n "$last_lines" ]; then
                performance_results[$backend_name]=$(echo "$last_lines" | tr '\n' '; ')
            else
                performance_results[$backend_name]="Performance test completed - no timing data captured"
            fi
        fi
    else
        performance_results[$backend_name]="Performance test failed or timed out"
    fi
    
    # Run comprehensive test
    echo "Running comprehensive tests..."
    if timeout 300 "$test_exe" > test_output.log 2>&1; then
        # Show the actual test output
        cat test_output.log
        
        if grep -q "ALL TESTS PASSED!" test_output.log; then
            test_results[$backend_name]="PASS"
        else
            test_results[$backend_name]="FAIL"
        fi
    else
        test_results[$backend_name]="TIMEOUT"
    fi
    
    # Memory analysis
    echo "Running memory analysis..."
    if command -v valgrind > /dev/null 2>&1; then
        valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
                 --error-exitcode=1 "$test_exe" > valgrind_output.log 2>&1
        
        if [ $? -eq 0 ]; then
            memory_results[$backend_name]="No memory leaks detected"
        else
            # Extract memory info
            definitely_lost=$(grep "definitely lost:" valgrind_output.log | grep -o "[0-9,]\+ bytes" | head -1 || echo "0 bytes")
            memory_results[$backend_name]="Memory issues detected - definitely lost: $definitely_lost"
        fi
    else
        memory_results[$backend_name]="Valgrind not available"
    fi
    
    cd ..
    echo ""
}

# Test all backends
for backend in nlohmann_json json11 rapidjson jsoncpp; do
    analyze_backend "$backend" "${backends[$backend]}"
done

# Print results summary
echo "============================================"
echo -e "${BLUE}COMPREHENSIVE ANALYSIS RESULTS${NC}"
echo "============================================"

echo ""
echo "Test Results:"
echo "-------------"
for backend in nlohmann_json json11 rapidjson jsoncpp; do
    result=${test_results[$backend]:-"NOT_TESTED"}
    if [ "$result" = "PASS" ]; then
        echo -e "  ${backend}: ${GREEN}✓ $result${NC}"
    else
        echo -e "  ${backend}: ${RED}✗ $result${NC}"
    fi
done

echo ""
echo "Performance Results:"
echo "--------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp; do
    result=${performance_results[$backend]:-"NOT_TESTED"}
    echo "  ${backend}: $result"
done

echo ""
echo "Memory Analysis Results:"
echo "------------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp; do
    result=${memory_results[$backend]:-"NOT_TESTED"}
    if [[ "$result" == "No memory leaks detected" ]]; then
        echo -e "  ${backend}: ${GREEN}✓ $result${NC}"
    elif [[ "$result" == *"issues detected"* ]]; then
        echo -e "  ${backend}: ${YELLOW}⚠️ $result${NC}"
    else
        echo -e "  ${backend}: ${RED}$result${NC}"
    fi
done

echo ""
echo "Code Metrics:"
echo "-------------"
UNIVERSAL_LINES=$(wc -l include/universal_observable_json.h 2>/dev/null | cut -d' ' -f1 || echo "0")
ADAPTER_LINES=$(wc -l include/universal_json_adapter.h 2>/dev/null | cut -d' ' -f1 || echo "0")

echo "Universal Observable JSON: ${UNIVERSAL_LINES} lines"
echo "Universal JSON Adapter: ${ADAPTER_LINES} lines"
echo "Total System: $((UNIVERSAL_LINES + ADAPTER_LINES)) lines"

echo ""
echo "Analysis completed for all backends: nlohmann/json, json11, RapidJSON, JsonCpp"

# Cleanup
rm -rf analysis_*
