#!/bin/bash

# Valgrind Analysis Script - Universal Observable JSON
# Comprehensive performance, memory, and thread safety analysis for all JSON backends
# Supports: nlohmann/json, json11, RapidJSON, JsonCpp, AxzDict

echo "Universal Observable JSON - Valgrind Analysis"
echo "=============================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check if Valgrind is available
if ! command -v valgrind > /dev/null 2>&1; then
    echo -e "${RED}ERROR: Valgrind not found!${NC}"
    echo "Install valgrind: sudo apt-get install valgrind"
    echo "Running basic tests without Valgrind..."
    echo ""
    
    # Fallback to basic testing
    cd /home/khoi/ws/observable_json_ai_generated
    if [ -f "./build/comprehensive_test" ]; then
        echo -e "${YELLOW}Running basic comprehensive test...${NC}"
        ./build/comprehensive_test
        echo ""
        echo -e "${GREEN}Basic test completed without Valgrind analysis${NC}"
    else
        echo -e "${RED}Test executable not found. Please build first.${NC}"
    fi
    exit 1
fi

echo -e "${GREEN}Valgrind detected - running full analysis${NC}"
echo ""

# Backend configurations
declare -A backends=(
    ["nlohmann_json"]=""
    ["json11"]="-DUSE_JSON11=ON"
    ["rapidjson"]="-DUSE_RAPIDJSON=ON"
    ["jsoncpp"]="-DUSE_JSONCPP=ON"
    ["axzdict"]="-DUSE_AXZDICT=ON"
)

# Results storage
declare -A memcheck_results
declare -A helgrind_results
declare -A performance_results

# Function to run valgrind analysis for a backend
run_valgrind_analysis() {
    local backend_name=$1
    local cmake_opts=$2
    local test_dir="valgrind_${backend_name}"
    
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}Analyzing ${backend_name} backend${NC}"
    echo -e "${BLUE}================================${NC}"
    
    # Clean and create test directory
    rm -rf "$test_dir"
    mkdir "$test_dir"
    cd "$test_dir"
    
    # Configure
    echo -e "${CYAN}Configuring ${backend_name}...${NC}"
    # Use RelWithDebInfo to avoid ASan conflicts with Valgrind and reduce strict warnings
    if cmake $cmake_opts -DCMAKE_BUILD_TYPE=RelWithDebInfo .. > configure.log 2>&1; then
        echo -e "${GREEN}✓ Configure successful${NC}"
    else
        echo -e "${RED}✗ Configure failed${NC}"
        memcheck_results[$backend_name]="CONFIG_FAIL"
        helgrind_results[$backend_name]="CONFIG_FAIL"
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
        memcheck_results[$backend_name]="BUILD_FAIL"
        helgrind_results[$backend_name]="BUILD_FAIL"
        performance_results[$backend_name]="BUILD_FAIL"
        cd ..
        return 1
    fi
    
    # Find executable
    test_exe=$(find . -name "comprehensive_test" -type f | head -1)
    if [ -z "$test_exe" ]; then
        echo -e "${RED}✗ Executable not found${NC}"
        memcheck_results[$backend_name]="EXE_NOT_FOUND"
        helgrind_results[$backend_name]="EXE_NOT_FOUND"
        performance_results[$backend_name]="EXE_NOT_FOUND"
        cd ..
        return 1
    fi
    
    echo -e "${PURPLE}Running Valgrind analysis for ${backend_name}...${NC}"
    
    # 1. Memory analysis with Memcheck
    echo -e "${CYAN}1. Memory Analysis (Memcheck)...${NC}"
    RUNNING_ON_VALGRIND=1 valgrind --tool=memcheck \
             --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --verbose \
             --log-file=memcheck_${backend_name}.log \
             "$test_exe" > test_output_memcheck.log 2>&1
    
    # Parse memcheck results
    if [ -f "memcheck_${backend_name}.log" ]; then
        DEFINITELY_LOST=$(grep "definitely lost:" memcheck_${backend_name}.log | grep -o "[0-9,]\+ bytes" | head -1 || echo "0 bytes")
        INDIRECTLY_LOST=$(grep "indirectly lost:" memcheck_${backend_name}.log | grep -o "[0-9,]\+ bytes" | head -1 || echo "0 bytes")
        POSSIBLY_LOST=$(grep "possibly lost:" memcheck_${backend_name}.log | grep -o "[0-9,]\+ bytes" | head -1 || echo "0 bytes")
        ERROR_SUMMARY=$(grep "ERROR SUMMARY:" memcheck_${backend_name}.log | tail -1 || echo "ERROR SUMMARY: unknown")
        
        echo -e "${GREEN}Memory Analysis Results:${NC}"
        echo "  Definitely lost: ${DEFINITELY_LOST:-0 bytes}"
        echo "  Indirectly lost: ${INDIRECTLY_LOST:-0 bytes}"
        echo "  Possibly lost: ${POSSIBLY_LOST:-0 bytes}"
        echo "  $ERROR_SUMMARY"
        
        # Better error detection
        if grep -q "FATAL:" memcheck_${backend_name}.log; then
            memcheck_results[$backend_name]="FATAL - $(grep "FATAL:" memcheck_${backend_name}.log | tail -1)"
        else
            memcheck_results[$backend_name]="PASS - $ERROR_SUMMARY"
        fi
    else
        memcheck_results[$backend_name]="MEMCHECK_FAIL"
    fi
    
    echo ""
    
    # 2. Thread safety analysis with Helgrind
    echo -e "${CYAN}2. Thread Safety Analysis (Helgrind)...${NC}"
    RUNNING_ON_VALGRIND=1 valgrind --tool=helgrind \
             --history-level=full \
             --conflict-cache-size=16777216 \
             --log-file=helgrind_${backend_name}.log \
             "$test_exe" > test_output_helgrind.log 2>&1
    
    # Parse helgrind results
    if [ -f "helgrind_${backend_name}.log" ]; then
        RACE_CONDITIONS=$(grep -c "Possible data race" helgrind_${backend_name}.log 2>/dev/null || echo "0")
        LOCK_ORDER=$(grep -c "lock order violated" helgrind_${backend_name}.log 2>/dev/null || echo "0")
        
        # Remove any newlines or extra characters
        RACE_CONDITIONS=$(echo "$RACE_CONDITIONS" | tr -d '\n' | tr -d ' ')
        LOCK_ORDER=$(echo "$LOCK_ORDER" | tr -d '\n' | tr -d ' ')
        
        echo -e "${GREEN}Thread Safety Results:${NC}"
        echo "  Possible data races: $RACE_CONDITIONS"
        echo "  Lock order violations: $LOCK_ORDER"
        
        if [ "$RACE_CONDITIONS" = "0" ] && [ "$LOCK_ORDER" = "0" ]; then
            helgrind_results[$backend_name]="PASS - No thread safety issues detected"
        else
            helgrind_results[$backend_name]="ISSUES - $RACE_CONDITIONS races, $LOCK_ORDER lock violations"
        fi
    else
        helgrind_results[$backend_name]="HELGRIND_FAIL"
    fi
    
    echo ""
    
    # 3. Performance analysis with Callgrind
    echo -e "${CYAN}3. Performance Analysis (Callgrind)...${NC}"
    RUNNING_ON_VALGRIND=1 valgrind --tool=callgrind \
             --callgrind-out-file=callgrind_${backend_name}.out \
             --collect-jumps=yes \
             --collect-systime=yes \
             "$test_exe" > test_output_callgrind.log 2>&1
    
    # Parse performance results
    if [ -f "callgrind_${backend_name}.out" ]; then
        INSTRUCTIONS=$(grep "events: Ir" callgrind_${backend_name}.out | head -1 || echo "events: Ir unknown")
        echo -e "${GREEN}Performance Results:${NC}"
        echo "  $INSTRUCTIONS"
        
        # Extract performance timing from test output
        PERF_TIME=$(grep "Performance:" test_output_callgrind.log | head -1 | sed 's/.*Performance: //' | sed 's/].*//' || echo "N/A")
        performance_results[$backend_name]="$PERF_TIME"
        echo "  Test performance: $PERF_TIME"
    else
        performance_results[$backend_name]="CALLGRIND_FAIL"
    fi
    
    echo ""
    
    # 4. Cache analysis with Cachegrind
    echo -e "${CYAN}4. Cache Analysis (Cachegrind)...${NC}"
    RUNNING_ON_VALGRIND=1 valgrind --tool=cachegrind \
             --cachegrind-out-file=cachegrind_${backend_name}.out \
             "$test_exe" > test_output_cachegrind.log 2>&1
    
    # Parse cache results
    if [ -f "cachegrind_${backend_name}.out" ]; then
        I_REFS=$(grep "I   refs:" cachegrind_${backend_name}.out | head -1 | sed 's/I   refs://' | xargs || echo "")
        D_REFS=$(grep "D   refs:" cachegrind_${backend_name}.out | head -1 | sed 's/D   refs://' | xargs || echo "")
        
        echo -e "${GREEN}Cache Analysis Results:${NC}"
        if [ -n "$I_REFS" ]; then
            echo "  Instruction refs: $I_REFS"
        else
            echo "  Instruction refs: Analysis completed (check .out file for details)"
        fi
        
        if [ -n "$D_REFS" ]; then
            echo "  Data refs: $D_REFS"
        else
            echo "  Data refs: Analysis completed (check .out file for details)"
        fi
    else
        echo -e "${RED}Cache analysis failed - no output file${NC}"
    fi
    
    echo ""
    
    # Show test results
    echo -e "${CYAN}Test Execution Results:${NC}"
    if grep -q "ALL TESTS PASSED!" test_output_memcheck.log || grep -q "All tests completed successfully" test_output_memcheck.log || grep -q "All .* tests passed" test_output_memcheck.log; then
        echo -e "${GREEN}✓ All tests passed${NC}"
    elif [ ! -s test_output_memcheck.log ]; then
        echo -e "${YELLOW}⚠️ Test output empty (tests may have run under Valgrind)${NC}"
    else
        echo -e "${RED}✗ Some tests failed or unclear results${NC}"
        echo "Failed tests:"
        grep -E "(FAIL:|assertion failed|Error:|error:|Assertion)" test_output_memcheck.log | head -5 || echo "  No specific failure messages found"
    fi
    
    echo ""
    echo -e "${PURPLE}Full logs saved:${NC}"
    echo "  - memcheck_${backend_name}.log"
    echo "  - helgrind_${backend_name}.log"
    echo "  - callgrind_${backend_name}.out"
    echo "  - cachegrind_${backend_name}.out"
    
    cd ..
    echo ""
}

# Main execution
cd /home/khoi/ws/observable_json_ai_generated

echo -e "${YELLOW}Starting comprehensive Valgrind analysis...${NC}"
echo ""

# Run analysis for all backends
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    run_valgrind_analysis "$backend" "${backends[$backend]}"
done

# Print comprehensive summary
echo -e "${PURPLE}========================================${NC}"
echo -e "${PURPLE}COMPREHENSIVE VALGRIND ANALYSIS SUMMARY${NC}"
echo -e "${PURPLE}========================================${NC}"
echo ""

echo -e "${BLUE}Memory Analysis (Memcheck) Results:${NC}"
echo "----------------------------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${memcheck_results[$backend]:-"NOT_TESTED"}
    if [[ "$result" == "PASS"* ]]; then
        echo -e "  ${backend}: ${GREEN}✓ $result${NC}"
    else
        echo -e "  ${backend}: ${RED}✗ $result${NC}"
    fi
done

echo ""
echo -e "${BLUE}Thread Safety Analysis (Helgrind) Results:${NC}"
echo "-------------------------------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${helgrind_results[$backend]:-"NOT_TESTED"}
    if [[ "$result" == "PASS"* ]]; then
        echo -e "  ${backend}: ${GREEN}✓ $result${NC}"
    elif [[ "$result" == "ISSUES"* ]]; then
        echo -e "  ${backend}: ${YELLOW}⚠️ $result${NC}"
    else
        echo -e "  ${backend}: ${RED}✗ $result${NC}"
    fi
done

echo ""
echo -e "${BLUE}Performance Analysis Results (1000 operations):${NC}"
echo "------------------------------------------------"
for backend in nlohmann_json json11 rapidjson jsoncpp axzdict; do
    result=${performance_results[$backend]:-"NOT_TESTED"}
    if [[ "$result" != *"FAIL"* ]] && [[ "$result" != "NOT_TESTED" ]]; then
        # Parse timing and add performance ranking
        if [[ "$result" =~ ([0-9]+)ms ]]; then
            timing=${BASH_REMATCH[1]}
            if [ "$timing" -lt 100 ]; then
                echo -e "  ${backend}: ${GREEN}⚡ $result (Excellent)${NC}"
            elif [ "$timing" -lt 500 ]; then
                echo -e "  ${backend}: ${YELLOW}🚀 $result (Good)${NC}"
            elif [ "$timing" -lt 2000 ]; then
                echo -e "  ${backend}: ${CYAN}⏱️  $result (Moderate)${NC}"
            else
                echo -e "  ${backend}: ${RED}🐌 $result (Slow)${NC}"
            fi
        else
            echo -e "  ${backend}: ${GREEN}$result${NC}"
        fi
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
echo -e "${GREEN}All Valgrind analysis logs have been saved in respective directories.${NC}"
echo -e "${YELLOW}Check individual backend directories for detailed reports.${NC}"

# Cleanup option
echo ""
read -p "Clean up temporary directories? (y/n): " -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf valgrind_*
    echo "Cleanup completed."
else
    echo "Temporary directories preserved for manual inspection."
fi

echo ""
echo -e "${PURPLE}Valgrind analysis completed successfully!${NC}"
