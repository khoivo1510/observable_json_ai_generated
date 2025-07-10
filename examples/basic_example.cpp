// BASIC EXAMPLE: Universal Observable JSON
// Demonstrates basic usage of the Universal Observable JSON library

#include "../include/universal_observable_json.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace universal_observable_json;

int main() {
    std::cout << "Universal Observable JSON - Basic Example\n";
    std::cout << "============================================\n\n";
    
    // 1. Create an observable JSON object
    UniversalObservableJson obs;
    
    // 2. Subscribe to changes
    auto subscription = obs.subscribe([](const json& new_value, const std::string& key, const json& old_value) {
        std::cout << "📡 Change detected!\n";
        std::cout << "   Key: " << key << "\n";
        std::cout << "   New value: " << json_adapter::dump(new_value) << "\n";
        std::cout << "   Old value: " << json_adapter::dump(old_value) << "\n\n";
    });
    
    // 3. Set some values
    std::cout << "Setting values...\n";
    obs.set("name", "Alice");
    obs.set("age", 30);
    obs.set("active", true);
    
    // Allow time for notifications
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 4. Get values
    std::cout << "Getting values...\n";
    std::cout << "Name: " << obs.get<std::string>("name") << "\n";
    std::cout << "Age: " << obs.get<int>("age") << "\n";
    std::cout << "Active: " << (obs.get<bool>("active") ? "true" : "false") << "\n\n";
    
    // 5. Check if keys exist
    std::cout << "Checking keys...\n";
    std::cout << "Has 'name': " << (obs.has("name") ? "yes" : "no") << "\n";
    std::cout << "Has 'email': " << (obs.has("email") ? "yes" : "no") << "\n\n";
    
    // 6. JSON serialization
    std::cout << "JSON representation:\n";
    std::cout << obs.dump(2) << "\n\n";
    
    // 7. Remove a key
    std::cout << "Removing 'age'...\n";
    obs.remove("age");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 8. Final state
    std::cout << "Final JSON:\n";
    std::cout << obs.dump(2) << "\n\n";
    
    // 9. Backend information
    std::cout << "Backend: " << json_adapter::get_backend_name() << "\n";
    std::cout << "Subscribers: " << obs.get_subscriber_count() << "\n";
    
    // 10. Unsubscribe
    obs.unsubscribe(subscription);
    
    std::cout << "\nExample completed successfully!\n";
    
    return 0;
}
