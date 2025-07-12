// Enhanced Universal Observable JSON Example
// Demonstrates advanced features and best practices

#include "../include/universal_observable_json.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

using namespace universal_observable_json;

int main() {
    std::cout << "Enhanced Universal Observable JSON Demo\n";
    std::cout << "=====================================\n\n";
    
    // 1. Create observable JSON with initial data
    UniversalObservableJson config(R"({
        "app": {
            "name": "MyApp",
            "version": "1.0.0",
            "debug": false
        },
        "server": {
            "host": "localhost",
            "port": 8080
        }
    })");
    
    std::cout << "1. Initial configuration:\n";
    std::cout << config.dump(2) << "\n\n";
    
    // 2. Subscribe to all changes
    auto global_sub = config.subscribe([](const json& new_val, const std::string& path, const json& old_val) {
        std::cout << "🔔 Global change: " << path << " = " << new_val.dump() << "\n";
    });
    
    // 3. Subscribe to specific path with debouncing
    auto debounced_sub = config.subscribe_debounced(
        [](const json& new_val, const std::string& path, const json& old_val) {
            std::cout << "⏱️  Debounced change: " << path << " = " << new_val.dump() << "\n";
        },
        std::chrono::milliseconds(50),
        "server" // Filter for server changes only
    );
    
    // 4. Demonstrate basic operations
    std::cout << "2. Setting values:\n";
    config.set("app_name", std::string("Enhanced App"));
    config.set("max_connections", 100);
    config.set("ssl_enabled", true);
    config.set("timeout", 30.5);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 5. Batch operations
    std::cout << "\n3. Batch operations:\n";
    config.set_batch({
        {"database_host", std::string("db.example.com")},
        {"database_port", 5432},
        {"database_name", std::string("myapp")},
        {"redis_enabled", false}
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 6. Async operations
    std::cout << "\n4. Async operations:\n";
    auto future1 = config.set_async("async_value", std::string("async_data"));
    auto future2 = config.get_async<std::string>("app_name");
    
    future1.wait();
    std::string app_name = future2.get();
    std::cout << "📦 Async retrieved app_name: " << app_name << "\n";
    
    // 7. Array-like operations
    std::cout << "\n5. Array-like operations:\n";
    config.push_back("features", std::string("authentication"));
    config.push_back("features", std::string("logging"));
    config.push_back("features", std::string("monitoring"));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 8. Type safety demonstration
    std::cout << "\n6. Type-safe access:\n";
    try {
        std::string name = config.get<std::string>("app_name");
        int max_conn = config.get<int>("max_connections");
        bool ssl = config.get<bool>("ssl_enabled");
        double timeout = config.get<double>("timeout");
        
        std::cout << "✅ Name: " << name << "\n";
        std::cout << "✅ Max connections: " << max_conn << "\n";
        std::cout << "✅ SSL enabled: " << (ssl ? "Yes" : "No") << "\n";
        std::cout << "✅ Timeout: " << timeout << "s\n";
    } catch (const std::exception& e) {
        std::cout << "❌ Error: " << e.what() << "\n";
    }
    
    // 9. Statistics and monitoring
    std::cout << "\n7. Statistics:\n";
    auto stats = config.get_statistics();
    std::cout << "📊 Active subscribers: " << stats.active_subscribers << "\n";
    std::cout << "📊 Data size: " << stats.data_size << " keys\n";
    std::cout << "📊 Pending notifications: " << stats.pending_notifications << "\n";
    
    // 10. Merge demonstration
    std::cout << "\n8. Merge operation:\n";
    UniversalObservableJson additional_config(R"({
        "cache_enabled": true,
        "cache_ttl": 3600,
        "log_level": "INFO"
    })");
    
    config.merge(additional_config);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 11. Final state
    std::cout << "\n9. Final configuration:\n";
    std::cout << config.dump(2) << "\n\n";
    
    // 12. Performance test
    std::cout << "10. Performance test:\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        config.set("perf_test_" + std::to_string(i), i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "⚡ 1000 operations completed in " << duration.count() << " μs\n";
    std::cout << "⚡ Average: " << (duration.count() / 1000.0) << " μs per operation\n";
    
    // 13. Thread safety test
    std::cout << "\n11. Thread safety test:\n";
    std::vector<std::thread> threads;
    std::atomic<int> total_changes{0};
    
    auto thread_sub = config.subscribe([&](const json&, const std::string&, const json&) {
        total_changes++;
    });
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&config, i]() {
            for (int j = 0; j < 10; ++j) {
                config.set("thread_" + std::to_string(i) + "_" + std::to_string(j), j);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "🧵 Completed " << total_changes.load() << " thread-safe operations\n";
    
    // 14. Cleanup
    config.unsubscribe(global_sub);
    config.unsubscribe(debounced_sub);
    config.unsubscribe(thread_sub);
    
    std::cout << "\n✨ Demo completed successfully!\n";
    std::cout << "🎯 Backend used: " << json_adapter::get_backend_name() << "\n";
    std::cout << "📈 Final data size: " << config.size() << " keys\n";
    
    return 0;
}
