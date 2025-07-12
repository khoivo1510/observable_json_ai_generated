#!/bin/bash

# Universal Backend Test Script
# Tests nlohmann/json, json11, RapidJSON, and JsonCpp backends

echo "Universal Observable JSON - Backend Testing"
echo "============================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results
declare -A results
declare -A performance

# Function to test a backend
test_backend() {
    local name=$1
    local cmake_opts=$2
    local test_dir="test_${name,,}"
    
    echo -e "${BLUE}Testing ${name} backend...${NC}"
    echo "------------------------------------------"
    
    # Clean and create test directory
    rm -rf "$test_dir"
    mkdir "$test_dir"
    cd "$test_dir"
    
    # Configure
    echo "Configuring..."
    if cmake $cmake_opts -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Configure OK${NC}"
    else
        echo -e "${RED}✗ Configure FAILED${NC}"
        results[$name]="FAIL"
        cd ..
        return 1
    fi
    
    # Build
    echo "Building..."
    if make comprehensive_test > /dev/null 2>&1; then
        echo -e "${GREEN}✓ Build OK${NC}"
    else
        echo -e "${RED}✗ Build FAILED${NC}"
        results[$name]="FAIL"
        cd ..
        return 1
    fi
    
    # Find executable
    test_exe=$(find . -name "comprehensive_test" -type f | head -1)
    if [ -z "$test_exe" ]; then
        echo -e "${RED}✗ Executable not found${NC}"
        results[$name]="FAIL"
        cd ..
        return 1
    fi
    
    # Run tests
    echo "Running tests..."
    if timeout 60 "$test_exe" > test_output.log 2>&1; then
        if grep -q "ALL TESTS PASSED!" test_output.log; then
            echo -e "${GREEN}✓ All tests PASSED${NC}"
            results[$name]="PASS"
            
    # Extract performance
    perf=$(grep "Performance:" test_output.log | head -1 | sed 's/.*Performance: //' | sed 's/].*//')
    if [ -n "$perf" ]; then
        performance[$name]=$perf
    else
        performance[$name]="N/A"
    fi
            
            # Show backend info
            backend_info=$(grep "Backend:" test_output.log | head -1 | sed 's/.*Backend: //' | sed 's/].*//')
            if [ -n "$backend_info" ]; then
                echo "Backend: $backend_info"
            fi
        else
            echo -e "${RED}✗ Some tests FAILED${NC}"
            results[$name]="FAIL"
            grep "FAIL:" test_output.log | head -3
        fi
    else
        echo -e "${RED}✗ Tests timeout/crash${NC}"
        results[$name]="FAIL"
    fi
    
    cd ..
    echo ""
}

# Test all backends
test_backend "nlohmann_json" ""
test_backend "json11" "-DUSE_JSON11=ON"  
test_backend "rapidjson" "-DUSE_RAPIDJSON=ON"
test_backend "jsoncpp" "-DUSE_JSONCPP=ON"

# Print summary
echo "============================================"
echo -e "${BLUE}BACKEND TEST SUMMARY${NC}"
echo "============================================"

echo "Results:"
for backend in nlohmann_json json11 rapidjson jsoncpp; do
    result=${results[$backend]:-"NOT_TESTED"}
    if [ "$result" = "PASS" ]; then
        echo -e "  ${backend}: ${GREEN}✓ PASS${NC}"
    else
        echo -e "  ${backend}: ${RED}✗ $result${NC}"
    fi
done

echo ""
echo "Performance (1000 operations):"
for backend in nlohmann_json json11 rapidjson jsoncpp; do
    perf=${performance[$backend]:-"N/A"}
    if [ "$perf" != "N/A" ]; then
        echo "  ${backend}: $perf"
    fi
done

# Count results
passed=0
total=0
for backend in nlohmann_json json11 rapidjson jsoncpp; do
    result=${results[$backend]:-"NOT_TESTED"}
    total=$((total + 1))
    if [ "$result" = "PASS" ]; then
        passed=$((passed + 1))
    fi
done

echo ""
echo "Summary: $passed/$total backends working"
if [ "$passed" -eq "$total" ]; then
    echo -e "${GREEN}🎉 ALL BACKENDS WORKING!${NC}"
else
    echo -e "${YELLOW}⚠️  Some backends need attention${NC}"
fi

cd ..

echo "Backend testing completed."
echo "Tested: nlohmann/json, json11, RapidJSON, JsonCpp"

# Cleanup
rm -rf test_nlohmann_json test_json11 test_rapidjson test_jsoncpp
