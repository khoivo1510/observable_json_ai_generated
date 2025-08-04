// PERFORMANCE COMPARISON: nlohmann/json vs json11
// Compares performance between different JSON backends

#include "../include/universal_observable_json.h"
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace universal_observable_json;

void benchmark_operations(const std::string& backend_name, int iterations = 10000) {
    std::cout << "\n🏃 Benchmarking " << backend_name << " backend:\n";
    std::cout << std::string(50, '-') << "\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Test 1: Object creation and destruction
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            UniversalObservableJson obs;
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        std::cout << "Object creation: " << duration.count() << " μs (" << iterations << " objects)\n";
    }
    
    // Test 2: Set operations
    {
        UniversalObservableJson obs;
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            obs.set("key" + std::to_string(i % 100), i);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        std::cout << "Set operations: " << duration.count() << " μs (" << iterations << " operations)\n";
    }
    
    // Test 3: Get operations
    {
        UniversalObservableJson obs;
        // Pre-populate
        for (int i = 0; i < 100; ++i) {
            obs.set("key" + std::to_string(i), i);
        }
        
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            try {
                auto val = obs.get<int>("key" + std::to_string(i % 100));
                (void)val; // Suppress unused variable warning
            } catch (...) {
                // Handle missing keys gracefully
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        std::cout << "Get operations: " << duration.count() << " μs (" << iterations << " operations)\n";
    }
    
    // Test 4: JSON serialization
    {
        UniversalObservableJson obs;
        // Pre-populate with complex data
        for (int i = 0; i < 50; ++i) {
            obs.set("string_" + std::to_string(i), "value_" + std::to_string(i));
            obs.set("number_" + std::to_string(i), i * 3.14);
            obs.set("bool_" + std::to_string(i), i % 2 == 0);
        }
        
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations / 100; ++i) {
            std::string json_str = obs.dump();
            (void)json_str; // Suppress unused variable warning
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        std::cout << "JSON serialization: " << duration.count() << " μs (" << iterations/100 << " operations)\n";
    }
    
    // Test 5: Subscription overhead
    {
        UniversalObservableJson obs;
        volatile int counter = 0;
        auto subscription = obs.subscribe([&counter](const json&, const std::string&, const json&) {
            counter++;
        });
        
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations / 10; ++i) {
            obs.set("key", i);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        std::cout << "Subscription notifications: " << duration.count() << " μs (" << iterations/10 << " operations)\n";
        
        obs.unsubscribe(subscription);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "\nTotal benchmark time: " << total_duration.count() << " ms\n";
}

int main() {
    std::cout << "UNIVERSAL OBSERVABLE JSON - PERFORMANCE COMPARISON\n";
    std::cout << "====================================================\n";
    
    std::cout << "Current backend: " << json_adapter::get_backend_name() << "\n";
    std::cout << "📖 Description: " << json_adapter::get_backend_description() << "\n";
    
    benchmark_operations(std::string(json_adapter::get_backend_name()));
    
    return 0;
}
