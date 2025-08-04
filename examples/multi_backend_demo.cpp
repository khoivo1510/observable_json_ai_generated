// 🌟 MULTI-BACKEND DEMO
// Demonstrates Universal Observable JSON with all supported backends

#include "../include/universal_observable_json.h"
#include <iostream>
#include <chrono>

using namespace universal_observable_json;

int main() {
    std::cout << "Universal Observable JSON - Multi-Backend Demo\n";
    std::cout << "=================================================\n\n";
    
    // Display current backend
    std::cout << "Current Backend: " << json_adapter::get_backend_name() << "\n";
    std::cout << "📖 Description: " << json_adapter::get_backend_description() << "\n\n";
    
    // Create observable JSON
    std::cout << "📋 Creating observable JSON object...\n";
    UniversalObservableJson obs;
    
    // Add subscription
    std::cout << "📡 Adding subscription...\n";
    volatile int notification_count = 0;
    auto sub = obs.subscribe([&notification_count](const json& new_val, const std::string& key, const json& /*old_val*/) {
        std::cout << "  🔔 Notification: " << key << " = " << json_adapter::dump(new_val) << "\n";
        notification_count++;
    });
    
    // Test operations
    std::cout << "\nTesting operations...\n";
    
    // Measure performance
    auto start = std::chrono::high_resolution_clock::now();
    
    // Set values
    obs.set("name", std::string("Universal JSON"));
    obs.set("version", std::string("1.0.0"));
    obs.set("backends", 3);
    obs.set("thread_safe", true);
    obs.set("performance", 95.5);
    
    // Get values
    std::cout << "\nCurrent values:\n";
    std::cout << "  Name: " << obs.get<std::string>("name") << "\n";
    std::cout << "  Version: " << obs.get<std::string>("version") << "\n";
    std::cout << "  Backends: " << obs.get<int>("backends") << "\n";
    std::cout << "  Thread Safe: " << (obs.get<bool>("thread_safe") ? "Yes" : "No") << "\n";
    std::cout << "  Performance Score: " << obs.get<double>("performance") << "\n";
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // JSON serialization
    std::cout << "\n📄 JSON Representation:\n";
    std::cout << obs.dump(2) << "\n\n";
    
    // Statistics
    std::cout << "📈 Statistics:\n";
    std::cout << "  Performance: " << duration.count() << " μs\n";
    std::cout << "  Notifications: " << notification_count << "\n";
    std::cout << "  Subscribers: " << obs.get_subscriber_count() << "\n";
    
    // Backend comparison
    std::cout << "\nBackend Comparison:\n";
    std::cout << "  Build with different backends:\n";
    std::cout << "    nlohmann/json: cmake -DUSE_JSON11=OFF -DUSE_RAPIDJSON=OFF ..\n";
    std::cout << "    json11:       cmake -DUSE_JSON11=ON ..\n";
    std::cout << "    RapidJSON:    cmake -DUSE_RAPIDJSON=ON ..\n";
    
    // Cleanup
    obs.unsubscribe(sub);
    
    std::cout << "\nDemo completed successfully!\n";
    return 0;
}
