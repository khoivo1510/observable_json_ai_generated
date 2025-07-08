#!/bin/bash

echo "🚀 OBSERVABLE JSON TEST SUITE"
echo "============================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test type selection
TEST_TYPE=${1:-"all"}

# Check if valgrind is installed
if ! command -v valgrind &> /dev/null; then
    echo -e "${YELLOW}⚠️  Valgrind not found! Install with: sudo apt-get install valgrind${NC}"
    VALGRIND_AVAILABLE=false
else
    VALGRIND_AVAILABLE=true
fi

# Check if binary exists
if [ ! -f "./build/ultimate_demo" ]; then
    echo -e "${RED}❌ Binary not found! Building...${NC}"
    cmake --build build
    if [ $? -ne 0 ]; then
        echo -e "${RED}❌ Build failed!${NC}"
        exit 1
    fi
fi

# Function to run basic test
run_basic_test() {
    echo -e "${BLUE}🧪 Basic Functionality Test${NC}"
    echo "==========================="
    ./build/ultimate_demo > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✅ Basic functionality: PASSED${NC}"
        return 0
    else
        echo -e "${RED}❌ Basic functionality: FAILED${NC}"
        return 1
    fi
}

# Function to run stability test
run_stability_test() {
    echo -e "${BLUE}🔄 Stability Test${NC}"
    echo "================="
    local TESTS=${1:-10}
    local PASSED=0
    
    for i in $(seq 1 $TESTS); do
        echo -n "Test $i/$TESTS: "
        if timeout 30s ./build/ultimate_demo >/dev/null 2>&1; then
            echo -e "${GREEN}✅ PASSED${NC}"
            ((PASSED++))
        else
            echo -e "${RED}❌ FAILED${NC}"
        fi
    done
    
    echo ""
    echo -e "${GREEN}📊 Results: $PASSED/$TESTS passed ($(( PASSED * 100 / TESTS ))%)${NC}"
    return $(( TESTS - PASSED ))
}

# Function to run memory check
run_memory_check() {
    if [ "$VALGRIND_AVAILABLE" != true ]; then
        echo -e "${YELLOW}⚠️  Memory check skipped (valgrind not available)${NC}"
        return 0
    fi
    
    echo -e "${BLUE}🔍 Memory Leak Check${NC}"
    echo "===================="
    echo "⏳ This may take a few minutes..."
    
    # Run valgrind and capture both stdout and stderr
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
             --error-exitcode=1 \
             ./build/ultimate_demo > valgrind_stdout.log 2> valgrind_memory.log
    
    EXIT_CODE=$?
    
    if [ $EXIT_CODE -eq 0 ]; then
        echo -e "${GREEN}✅ Memory check: NO LEAKS DETECTED${NC}"
        
        # Show summary if available
        if [ -s valgrind_memory.log ]; then
            echo -e "${BLUE}📄 Memory Summary:${NC}"
            grep -E "(HEAP SUMMARY|LEAK SUMMARY|ERROR SUMMARY)" valgrind_memory.log | head -10
        fi
        return 0
    else
        echo -e "${RED}❌ Memory check: LEAKS FOUND${NC}"
        echo "📄 Check valgrind_memory.log for details"
        
        if [ -s valgrind_memory.log ]; then
            echo -e "${RED}🔍 Key Issues:${NC}"
            grep -E "(ERROR SUMMARY|definitely lost|indirectly lost|possibly lost)" valgrind_memory.log | head -5
        else
            echo "⚠️  Log file is empty, valgrind may have failed"
        fi
        return 1
    fi
}

# Function to run threading check
run_threading_check() {
    if [ "$VALGRIND_AVAILABLE" != true ]; then
        echo -e "${YELLOW}⚠️  Threading check skipped (valgrind not available)${NC}"
        return 0
    fi
    
    echo -e "${BLUE}🧵 Threading Safety Check${NC}"
    echo "========================="
    echo "⏳ This may take several minutes..."
    
    # Run helgrind
    valgrind --tool=helgrind --error-exitcode=1 \
             ./build/ultimate_demo > helgrind_stdout.log 2> valgrind_threading.log
    
    EXIT_CODE=$?
    
    if [ $EXIT_CODE -eq 0 ]; then
        echo -e "${GREEN}✅ Threading check: NO ISSUES DETECTED${NC}"
        
        # Show summary if available
        if [ -s valgrind_threading.log ]; then
            echo -e "${BLUE}📄 Threading Summary:${NC}"
            grep -E "(ERROR SUMMARY|Possible data race)" valgrind_threading.log | head -3
        fi
        return 0
    else
        echo -e "${RED}❌ Threading check: ISSUES FOUND${NC}"
        echo "📄 Check valgrind_threading.log for details"
        
        if [ -s valgrind_threading.log ]; then
            echo -e "${RED}🔍 Key Issues:${NC}"
            grep -E "(ERROR SUMMARY|Possible data race|Lock order violated)" valgrind_threading.log | head -5
            echo ""
            echo -e "${YELLOW}💡 Common threading issues:${NC}"
            echo "  - Data races: Multiple threads accessing same memory without synchronization"
            echo "  - Lock order violations: Different lock acquisition order can cause deadlocks"
        else
            echo "⚠️  Log file is empty, helgrind may have failed"
        fi
        return 1
    fi
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [test_type]"
    echo ""
    echo "Test types:"
    echo "  basic      - Basic functionality test only"
    echo "  stability  - Stability test (10 runs)"
    echo "  memory     - Memory leak check with valgrind"
    echo "  threading  - Threading safety check with helgrind"
    echo "  all        - All tests (default)"
    echo ""
    echo "Examples:"
    echo "  $0                # Run all tests"
    echo "  $0 basic          # Quick basic test"
    echo "  $0 stability      # Only stability test"
    echo "  $0 memory         # Only memory check"
}

# Main execution
case $TEST_TYPE in
    "basic")
        run_basic_test
        exit $?
        ;;
    "stability")
        run_stability_test 10
        exit $?
        ;;
    "memory")
        run_memory_check
        exit $?
        ;;
    "threading")
        run_threading_check
        exit $?
        ;;
    "all")
        echo ""
        TOTAL_FAILURES=0
        
        # Run all tests
        run_basic_test || ((TOTAL_FAILURES++))
        echo ""
        
        run_stability_test 5 || ((TOTAL_FAILURES++))
        echo ""
        
        run_memory_check || ((TOTAL_FAILURES++))
        echo ""
        
        run_threading_check || ((TOTAL_FAILURES++))
        echo ""
        
        if [ $TOTAL_FAILURES -eq 0 ]; then
            echo -e "${GREEN}🎉 ALL TESTS PASSED!${NC}"
            exit 0
        else
            echo -e "${RED}❌ $TOTAL_FAILURES test(s) failed${NC}"
            exit 1
        fi
        ;;
    "help"|"-h"|"--help")
        show_usage
        exit 0
        ;;
    *)
        echo -e "${RED}❌ Unknown test type: $TEST_TYPE${NC}"
        show_usage
        exit 1
        ;;
esac
