#!/bin/bash

# Universal Backend Test Script - Universal Observable JSON
# Tests nlohmann/json, json11, RapidJSON, JsonCpp, and AxzDict backends

echo "Universal Observable JSON - Backend Testing"
echo "============================================"

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

# Function to test a backend
test_backend() {
    local backend_name=$1
    local cmake_opts=$2
    local test_dir="test_${backend_name}"
    
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}Testing ${backend_name} backend${NC}"
    echo -e "${BLUE}================================${NC}"
    
    # Clean and create test directory
    rm -rf "$test_dir"
    mkdir "$test_dir"
    cd "$test_dir"
    
    # Configure
    echo -e "${CYAN}Configuring ${backend_name}...${NC}"
    if cmake $cmake_opts -DCMAKE_BUILD_TYPE=Release .. > configure.log 2>&1; then
        echo -e "${GREEN}✓ Configure successful${NC}"
    else
        echo -e "${RED}✗ Configure failed${NC}"
        test_results[$backend_name]="CONFIG_FAIL"
        performance_results[$backend_name]="CONFIG_FAIL"
        cd ..
        return 1
    fi
    
    # Build
    echo -e "${CYAN}Building ${backend_name}...${NC}"
    if make comprehensive_test > build.log 2>&1; then
        echo -e "${GREEN}✓ Build successful${NC}"
    else
        echo -e "${RED}✗ Build failed${NC}"
        test_results[$backend_name]="BUILD_FAIL"
        performance_results[$backend_name]="BUILD_FAIL"
        cd ..
        return 1
    fi
    
    # Find executable
    test_exe=$(find . -name "comprehensive_test" -type f | head -1)
    if [ -z "$test_exe" ]; then
        echo -e "${RED}✗ Executable not found${NC}"
        test_results[$backend_name]="EXE_NOT_FOUND"
        performance_results[$backend_name]="EXE_NOT_FOUND"
        cd ..
        return 1
    fi
    
    # Run tests
    echo -e "${CYAN}Running comprehensive tests...${NC}"
    if timeout 300 "$test_exe" > test_output.log 2>&1; then
        if grep -q "ALL TESTS PASSED!" test_output.log; then
            echo -e "${GREEN}✓ All tests passed${NC}"
            test_results[$backend_name]="PASS"
            
            # Extract performance data
            perf=$(grep "Performance:" test_output.log | head -1 | sed 's/.*Performance: //' | sed 's/].*//')
            if [ -n "$perf" ]; then
                performance_results[$backend_name]="$perf"
            else
                performance_results[$backend_name]="N/A"
            fi
            
            # Show backend info
            backend_info=$(grep "Backend:" test_output.log | head -1 | sed 's/.*Backend: //' | sed 's/].*//')
            if [ -n "$backend_info" ]; then
                echo -e "${GREEN}Backend confirmed: $backend_info${NC}"
            fi
        else
            echo -e "${RED}✗ Some tests failed${NC}"
            test_results[$backend_name]="FAIL"
            performance_results[$backend_name]="FAIL"
            
            # Show failed test summary
            failed_count=$(grep -c "FAIL:" test_output.log || echo "0")
            if [ "$failed_count" -gt "0" ]; then
                echo -e "${RED}Failed tests: $failed_count${NC}"
            fi
        fi
    else
        echo -e "${RED}✗ Tests timeout or crashed${NC}"
        test_results[$backend_name]="TIMEOUT"
        performance_results[$backend_name]="TIMEOUT"
    fi
    
    cd ..
    echo ""
}

# Main execution
echo -e "${YELLOW}Starting backend testing...${NC}"
echo ""

# Test all backends
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    test_backend "$backend" "${backends[$backend]}"
done

# Print summary
echo -e "${PURPLE}========================================${NC}"
echo -e "${PURPLE}BACKEND TEST SUMMARY${NC}"
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
echo -e "${BLUE}Performance Results (1000 operations):${NC}"
echo "---------------------------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    perf=${performance_results[$backend]:-"N/A"}
    if [[ "$perf" != *"FAIL"* ]] && [[ "$perf" != "NOT_TESTED" ]] && [ "$perf" != "N/A" ]; then
        echo -e "  ${backend}: ${GREEN}$perf${NC}"
    else
        echo -e "  ${backend}: ${RED}$perf${NC}"
    fi
done

# Count results
passed=0
total=0
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${test_results[$backend]:-"NOT_TESTED"}
    total=$((total + 1))
    if [ "$result" = "PASS" ]; then
        passed=$((passed + 1))
    fi
done

echo ""
echo -e "${BLUE}Summary: $passed/$total backends working${NC}"
if [ "$passed" -eq "$total" ]; then
    echo -e "${GREEN}🎉 ALL BACKENDS WORKING!${NC}"
else
    echo -e "${YELLOW}⚠️ Some backends need attention${NC}"
fi

echo ""
echo -e "${BLUE}Backend testing completed for all backends:${NC}"
echo "  - nlohmann/json: Full-featured JSON library"
echo "  - json11: Lightweight JSON library"
echo "  - RapidJSON: Fast JSON parser/generator"
echo "  - JsonCpp: Mature JSON library"
echo "  - AxzDict: Advanced dictionary-based JSON implementation"

echo ""
echo -e "${GREEN}All test logs have been saved in respective directories.${NC}"

# Cleanup option
echo ""
read -p "Clean up temporary directories? (y/n): " -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf test_*
    echo "Cleanup completed."
else
    echo "Temporary directories preserved for manual inspection."
fi

echo ""
echo -e "${PURPLE}Backend testing completed successfully!${NC}"
