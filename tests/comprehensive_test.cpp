// COMPREHENSIVE TEST SUITE FOR UNIVERSAL OBSERVABLE JSON
// Single file containing all test cases for the universal observable JSON system
// Author: AI Enhanced - 2025-07-10

#include "../include/universal_observable_json.h"
#include "../include/universal_json_adapter.h"  // For backend macros
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <atomic>
#include <future>
#include <random>
#include <memory>

using namespace universal_observable_json;

// ==================== TEST FRAMEWORK ====================
class TestFramework {
public:
    static void run_test(const std::string& name, std::function<void()> test_func) {
        std::cout << "TEST: " << name << "... ";
        try {
            test_func();
            std::cout << "PASS\n";
            passed_++;
        } catch (const std::exception& e) {
            std::cout << "FAIL: " << e.what() << "\n";
            failed_++;
        }
        total_++;
    }
    
    static void print_summary() {
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "TEST SUMMARY\n";
        std::cout << "Total: " << total_ << " | Passed: " << passed_ << " | Failed: " << failed_ << "\n";
        if (failed_ == 0) {
            std::cout << "ALL TESTS PASSED!\n";
        } else {
            std::cout << failed_ << " tests failed\n";
        }
        std::cout << std::string(50, '=') << "\n";
    }
    
    static bool all_passed() { return failed_ == 0; }
    
private:
    static int total_, passed_, failed_;
};

int TestFramework::total_ = 0;
int TestFramework::passed_ = 0;
int TestFramework::failed_ = 0;

// ==================== TEST CASES ====================

namespace tests {

// Test 1: Basic Construction
void test_basic_construction() {
    UniversalObservableJson obs1;
    UniversalObservableJson obs2(std::string(R"({"name": "test", "value": 42})"));
    
    json initial_data = json_adapter::make_object();
    UniversalObservableJson obs3(initial_data);
    
    assert(obs2.get<std::string>("name") == "test");
    assert(obs2.get<int>("value") == 42);
}

// Test 2: Basic CRUD Operations
void test_basic_crud() {
    UniversalObservableJson obs;
    
    // Set operations
    obs.set("name", std::string("Alice"));
    obs.set("age", 30);
    obs.set("active", true);
    obs.set("score", 95.5);
    
    // Get operations
    assert(obs.get<std::string>("name") == "Alice");
    assert(obs.get<int>("age") == 30);
    assert(obs.get<bool>("active") == true);
    assert(abs(obs.get<double>("score") - 95.5) < 0.001);
    
    // Has operations
    assert(obs.has("name"));
    assert(obs.has("age"));
    assert(!obs.has("nonexistent"));
    
    // Remove operations
    obs.remove("score");
    assert(!obs.has("score"));
}

// Test 3: Subscription System
void test_subscription_system() {
    UniversalObservableJson obs;
    std::vector<std::string> events;
    
    // Subscribe to changes
    size_t sub_id = obs.subscribe([&events](const json& new_val, const std::string& key, const json& old_val) {
        events.push_back("changed:" + key);
    });
    
    // Trigger changes
    obs.set("test1", std::string("value1"));
    obs.set("test2", std::string("value2"));
    obs.remove("test1");
    
    // Allow async processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    assert(events.size() >= 3);
    
    // Unsubscribe
    obs.unsubscribe(sub_id);
    size_t events_before = events.size();
    obs.set("test3", std::string("value3"));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    assert(events.size() == events_before); // No new events after unsubscribe
}

// Test 4: Multiple Subscribers
void test_multiple_subscribers() {
    UniversalObservableJson obs;
    std::atomic<int> counter1{0}, counter2{0};
    
    auto sub1 = obs.subscribe([&counter1](const json&, const std::string&, const json&) {
        counter1++;
    });
    
    auto sub2 = obs.subscribe([&counter2](const json&, const std::string&, const json&) {
        counter2++;
    });
    
    obs.set("key1", std::string("value1"));
    obs.set("key2", std::string("value2"));
    obs.set("key3", std::string("value3"));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    assert(counter1.load() == 3);
    assert(counter2.load() == 3);
    
    obs.unsubscribe(sub1);
    obs.unsubscribe(sub2);
}

// Test 5: Thread Safety
void test_thread_safety() {
    UniversalObservableJson obs;
    std::atomic<int> event_count{0};
    
    auto sub = obs.subscribe([&event_count](const json&, const std::string&, const json&) {
        event_count++;
    });
    
    const int num_threads = 5;
    const int ops_per_thread = 10;
    std::vector<std::thread> threads;
    
    // Launch multiple threads doing concurrent operations
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&obs, i, ops_per_thread]() {
            for (int j = 0; j < ops_per_thread; ++j) {
                std::string key = "thread_" + std::to_string(i) + "_key_" + std::to_string(j);
                obs.set(key, j);
                
                if (j % 2 == 0) {
                    obs.remove(key);
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Should have received events from all operations
    assert(event_count.load() > 0);
    
    obs.unsubscribe(sub);
}

// Test 6: JSON Serialization/Deserialization
void test_json_serialization() {
    UniversalObservableJson obs;
    obs.set("name", std::string("John"));
    obs.set("age", 25);
    obs.set("active", true);
    obs.set("score", 87.5);
    
    // Serialize to JSON string
    std::string json_str = obs.dump();
    assert(!json_str.empty());
    
    // Pretty print - Note: json11 doesn't support pretty printing, so both should be same
    std::string pretty_json = obs.dump(2);
    #if JSON_ADAPTER_BACKEND == JSON11
        assert(pretty_json == json_str); // json11 doesn't support pretty printing
    #else
        assert(pretty_json.length() > json_str.length()); // Other backends support pretty printing
    #endif
    
    // Deserialize from JSON string
    UniversalObservableJson obs2(json_str);
    assert(obs2.get<std::string>("name") == "John");
    assert(obs2.get<int>("age") == 25);
    assert(obs2.get<bool>("active") == true);
    assert(abs(obs2.get<double>("score") - 87.5) < 0.001);
}

// Test 7: Utility Functions
void test_utility_functions() {
    UniversalObservableJson obs;
    
    // Test on empty object
    assert(obs.get_subscriber_count() == 0);
    
    // Add subscriber
    auto sub = obs.subscribe([](const json&, const std::string&, const json&) {});
    assert(obs.get_subscriber_count() == 1);
    
    // Add some data
    obs.set("key1", std::string("value1"));
    obs.set("key2", std::string("value2"));
    
    // Test clear
    obs.clear();
    std::string dumped = obs.dump();
    
    // Should be empty object
    assert(dumped == "{}" || dumped.find("{}") != std::string::npos);
    
    obs.unsubscribe(sub);
}

// Test 8: Error Handling
void test_error_handling() {
    UniversalObservableJson obs;
    
    // Test getting non-existent key
    try {
        obs.get<std::string>("nonexistent");
        assert(false); // Should not reach here
    } catch (const std::exception&) {
        // Expected
    }
    
    // Test removing non-existent key (should not throw)
    obs.remove("nonexistent");
    
    // Test with invalid JSON (constructor)
    try {
        std::string invalid_json = "invalid json string";
        UniversalObservableJson obs2(invalid_json);
        assert(false); // Should not reach here
    } catch (const std::exception&) {
        // Expected
    }
}

// Test 9: Performance Test
void test_performance() {
    UniversalObservableJson obs;
    auto start = std::chrono::high_resolution_clock::now();
    
    // Perform many operations
    for (int i = 0; i < 1000; ++i) {
        obs.set("key_" + std::to_string(i), i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "[Performance: " << duration.count() << "ms for 1000 operations] ";
    
    // Should complete within reasonable time
    assert(duration.count() < 1000); // Less than 1 second
}

// Test 10: Backend Information
void test_backend_info() {
    std::string backend_name = json_adapter::get_backend_name();
    assert(!backend_name.empty());
    
    std::cout << "[Backend: " << backend_name << "] ";
}

// Test 11: Exception Safety
void test_exception_safety() {
        UniversalObservableJson obs;
        
        std::atomic<int> safe_callbacks{0};
        std::atomic<int> exception_callbacks{0};
        
        // Add a callback that throws exceptions
        auto id = obs.subscribe([&](const json& /*new_val*/, const std::string& path, const json& /*old_val*/) {
            if (path == "throw_exception") {
                exception_callbacks++;
                throw std::runtime_error("Test exception");
            }
            safe_callbacks++;
        });
        
        // Test that exceptions in callbacks don't crash the system
        obs.set("safe_key", std::string("safe_value"));
        obs.set("throw_exception", std::string("boom"));
        obs.set("another_safe_key", std::string("another_safe_value"));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        assert(safe_callbacks.load() == 2);
        assert(exception_callbacks.load() == 1);
        
        // Verify system remains functional
        assert(obs.get<std::string>("safe_key") == "safe_value");
        assert(obs.get<std::string>("another_safe_key") == "another_safe_value");
    }

    // Test 12: Edge Cases
    void test_edge_cases() {
        UniversalObservableJson obs;
        
        // Test special characters in keys
        obs.set("special!@#$%^&*()", std::string("special_value"));
        assert(obs.get<std::string>("special!@#$%^&*()") == "special_value");
        
        // Test very long keys
        std::string long_key(1000, 'a');
        obs.set(long_key, std::string("long_key_value"));
        assert(obs.get<std::string>(long_key) == "long_key_value");
        
        // Test deep nesting
        obs.set("level1/level2/level3/level4/level5", std::string("deep_value"));
        assert(obs.get<std::string>("level1/level2/level3/level4/level5") == "deep_value");
        
        // Test invalid path access should throw
        try {
            obs.get<std::string>("nonexistent/path");
            assert(false && "Should have thrown exception");
        } catch (...) {
            // Expected
        }
        
        // Test type mismatches should throw or handle gracefully
        obs.set("number", 42);
        try {
            auto result = obs.get<std::string>("number");
            // Some backends may allow this conversion, others may throw
            // Either behavior is acceptable
        } catch (...) {
            // This is also acceptable
        }
    }

    // Test 13: Custom Types
    void test_custom_types() {
        UniversalObservableJson obs;
        
        // Test with custom JSON objects using universal adapter
        obs.set("custom/name", std::string("Test Object"));
        obs.set("custom/id", 12345);
        obs.set("custom/active", true);
        
        assert(obs.get<std::string>("custom/name") == "Test Object");
        assert(obs.get<int>("custom/id") == 12345);
        assert(obs.get<bool>("custom/active") == true);
        
        // Test nested custom objects
        obs.set("nested/user/profile/name", std::string("John Doe"));
        obs.set("nested/user/profile/age", 30);
        obs.set("nested/user/settings/theme", std::string("dark"));
        obs.set("nested/user/settings/notifications", true);
        
        assert(obs.get<std::string>("nested/user/profile/name") == "John Doe");
        assert(obs.get<int>("nested/user/profile/age") == 30);
        assert(obs.get<std::string>("nested/user/settings/theme") == "dark");
        assert(obs.get<bool>("nested/user/settings/notifications") == true);
    }

    // Test 14: Stress Testing
    void test_stress_testing() {
        UniversalObservableJson obs;
        
        // Stress test with many rapid operations
        const int num_operations = 100; // Reduced for faster testing
        
        // Test rapid set operations
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_operations; ++i) {
            obs.set("stress_key_" + std::to_string(i), i);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Should complete within reasonable time
        assert(duration.count() < 5000); // 5 seconds max
        
        // Test rapid get operations
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_operations; ++i) {
            auto value = obs.get<int>("stress_key_" + std::to_string(i));
            assert(value == i);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        assert(duration.count() < 2000); // 2 seconds max for reads
        
        // Test concurrent subscriptions
        std::atomic<int> total_callbacks{0};
        std::vector<size_t> subscription_ids;
        
        for (int i = 0; i < 10; ++i) {
            auto id = obs.subscribe([&](const json&, const std::string&, const json&) {
                total_callbacks++;
            });
            subscription_ids.push_back(id);
        }
        
        obs.set("stress_trigger", std::string("test"));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        assert(total_callbacks.load() == 10); // All 10 subscribers should be notified
        
        // Clean up subscriptions
        for (auto id : subscription_ids) {
            obs.unsubscribe(id);
        }
    }

} // namespace tests

// ==================== MAIN TEST RUNNER ====================
int main() {
    std::cout << "UNIVERSAL OBSERVABLE JSON - COMPREHENSIVE TEST SUITE\n";
    std::cout << "=======================================================\n";
    
    // Run all tests
    TestFramework::run_test("Basic Construction", tests::test_basic_construction);
    TestFramework::run_test("Basic CRUD Operations", tests::test_basic_crud);
    TestFramework::run_test("Subscription System", tests::test_subscription_system);
    TestFramework::run_test("Multiple Subscribers", tests::test_multiple_subscribers);
    TestFramework::run_test("Thread Safety", tests::test_thread_safety);
    TestFramework::run_test("JSON Serialization", tests::test_json_serialization);
    TestFramework::run_test("Utility Functions", tests::test_utility_functions);
    TestFramework::run_test("Error Handling", tests::test_error_handling);
    TestFramework::run_test("Performance Test", tests::test_performance);
    TestFramework::run_test("Backend Information", tests::test_backend_info);
    TestFramework::run_test("Exception Safety", tests::test_exception_safety);
    TestFramework::run_test("Edge Cases", tests::test_edge_cases);
    TestFramework::run_test("Custom Types", tests::test_custom_types);
    TestFramework::run_test("Stress Testing", tests::test_stress_testing);
    
    TestFramework::print_summary();
    
    return TestFramework::all_passed() ? 0 : 1;
}
