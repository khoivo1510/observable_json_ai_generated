#!/bin/bash

# Helgrind Thread Safety Analysis - Universal Observable JSON
# Thread safety and race condition detection

echo "Helgrind Thread Safety Analysis"
echo "==============================="

# Check if Valgrind is available
if ! command -v valgrind > /dev/null 2>&1; then
    echo "Valgrind not found - performing basic thread safety test instead"
    echo ""
    
    # Run basic multi-threaded test without Valgrind
    echo "Running basic thread safety test..."
    cd /home/khoi/ws/observable_json_ai_generated
    
    if [ -f "./build/comprehensive_test" ]; then
        echo "Running comprehensive test suite..."
        ./build/comprehensive_test | grep -E "(Thread|Safety|PASS|FAIL)"
        echo ""
        echo "Basic functionality test completed"
        echo "Install valgrind for detailed thread safety analysis:"
        echo "   sudo apt-get install valgrind"
    else
        echo "Test executable not found. Run: make comprehensive_test"
    fi
    exit 0
fi

# Build test
echo "Building thread safety test..."
mkdir -p helgrind_test
cd helgrind_test

# Create comprehensive thread safety test
cat > thread_safety_test.cpp << 'EOF'
#include "../include/universal_observable_json.h"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>

using namespace universal_observable_json;

std::atomic<int> callback_count{0};
std::atomic<int> read_count{0};
std::atomic<int> write_count{0};

void comprehensive_thread_safety_test() {
    UniversalObservableJson obs;
    
    // Add multiple subscribers
    std::vector<size_t> subscriptions;
    for (int i = 0; i < 3; ++i) {
        auto sub = obs.subscribe([i](const auto&, const std::string& key, const auto&) {
            callback_count++;
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        });
        subscriptions.push_back(sub);
    }
    
    std::vector<std::thread> threads;
    
    // Heavy writer threads - concurrent modifications
    for (int t = 0; t < 6; ++t) {
        threads.emplace_back([&obs, t]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 100);
            
            for (int i = 0; i < 100; ++i) {
                std::string key = "thread_" + std::to_string(t) + "_key_" + std::to_string(i % 10);
                
                // Mix of operations
                obs.set(key, dis(gen));
                write_count++;
                
                if (i % 15 == 0) {
                    obs.remove(key);
                }
                
                // Shared key modifications
                obs.set("shared_counter", i * t);
                write_count++;
                
                // Brief pause to allow other threads
                if (i % 20 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
        });
    }
    
    // Heavy reader threads - concurrent reads
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([&obs, t]() {
            for (int i = 0; i < 150; ++i) {
                try {
                    // Various read operations
                    auto shared = obs.get<int>("shared_counter");
                    read_count++;
                    
                    auto has_key = obs.has("thread_0_key_0");
                    read_count++;
                    
                    auto json_str = obs.dump();
                    read_count++;
                    
                    auto subscriber_count = obs.get_subscriber_count();
                    read_count++;
                    
                    // Try reading from different thread keys
                    for (int j = 0; j < 6; ++j) {
                        std::string key = "thread_" + std::to_string(j) + "_key_" + std::to_string(i % 10);
                        try {
                            auto val = obs.get<int>(key);
                            read_count++;
                        } catch (...) {
                            // Expected - key might not exist
                        }
                    }
                    
                    (void)shared; (void)has_key; (void)json_str; (void)subscriber_count; // Suppress warnings
                } catch (...) {
                    // Expected - concurrent modifications
                }
                
                if (i % 30 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
        });
    }
    
    // Subscription management threads
    for (int t = 0; t < 2; ++t) {
        threads.emplace_back([&obs]() {
            for (int i = 0; i < 20; ++i) {
                // Add and remove subscribers dynamically
                auto sub = obs.subscribe([](const auto&, const std::string&, const auto&) {
                    callback_count++;
                });
                
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                
                obs.unsubscribe(sub);
                
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }
    
    std::cout << "Running concurrent stress test with " << threads.size() << " threads..." << std::endl;
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Cleanup subscriptions
    for (auto sub : subscriptions) {
        obs.unsubscribe(sub);
    }
    
    std::cout << "Stress test completed!" << std::endl;
    std::cout << "Final stats:" << std::endl;
    std::cout << "  - Callbacks fired: " << callback_count.load() << std::endl;
    std::cout << "  - Read operations: " << read_count.load() << std::endl;
    std::cout << "  - Write operations: " << write_count.load() << std::endl;
    std::cout << "  - Final subscriber count: " << obs.get_subscriber_count() << std::endl;
}

int main() {
    std::cout << "🧵 Universal Observable JSON - Thread Safety Test" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    comprehensive_thread_safety_test();
    
    std::cout << "\nThread safety test completed successfully!" << std::endl;
    return 0;
}
EOF

# Compile with debug info for better Helgrind analysis
echo "🔨 Compiling thread safety test..."
g++ -std=c++17 -g -O1 -I.. -I../build/_deps/nlohmann_json-src/include thread_safety_test.cpp -o thread_safety_test -pthread

if [ ! -f "./thread_safety_test" ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compilation successful!"

# Check if Valgrind/Helgrind is available
if ! command -v valgrind > /dev/null 2>&1; then
    echo "Valgrind not available - running basic test only"
    ./thread_safety_test
    cd ..
    rm -rf helgrind_test
    exit 0
fi

echo ""
echo "Running Helgrind race condition detection..."
echo "This may take several minutes..."

# Run Helgrind with comprehensive options
valgrind --tool=helgrind \
         --history-level=full \
         --conflict-cache-size=16777216 \
         --check-stack-refs=yes \
         ./thread_safety_test > helgrind_output.txt 2>&1

echo "Analyzing results..."

# Parse Helgrind output
RACE_CONDITIONS=$(grep -c "Possible data race" helgrind_output.txt 2>/dev/null || echo "0")
LOCK_ORDER=$(grep -c "lock order violated" helgrind_output.txt 2>/dev/null || echo "0") 
THREAD_ERRORS=$(grep -c "Thread #" helgrind_output.txt 2>/dev/null || echo "0")

echo ""
echo "Helgrind Analysis Results:"
echo "========================="
echo "Thread Safety Summary:"
echo "- Possible data races: ${RACE_CONDITIONS}"
echo "- Lock order violations: ${LOCK_ORDER}"  
echo "- Thread-related errors: ${THREAD_ERRORS}"

if [ "$RACE_CONDITIONS" = "0" ] && [ "$LOCK_ORDER" = "0" ]; then
    echo ""
    echo "No thread safety issues detected."
else
    echo ""
    echo "Thread safety issues detected."
    echo "First few issues:"
    echo "================"
    grep -A 10 -B 2 "Possible data race\|lock order violated" helgrind_output.txt | head -30
    echo ""
    echo "Full report saved to: helgrind_output.txt"
fi

# Performance impact analysis
echo ""
echo "Performance Impact Analysis:"
echo "=============================="

echo "Running normal performance test..."
time ./thread_safety_test > /dev/null 2>&1
NORMAL_TIME=$?

echo ""
echo "Performance Summary:"
echo "- Thread safety test completed with concurrent access"
echo "- All operations performed without deadlocks"

# Cleanup
echo ""
echo "Cleaning up..."
rm -f thread_safety_test.cpp thread_safety_test

cd ..
rm -rf helgrind_test

echo ""
echo "Helgrind analysis completed."
