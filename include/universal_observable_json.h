#pragma once

// UNIVERSAL OBSERVABLE JSON - Works with ANY JSON backend
// Uses the universal JSON adapter to support all major JSON libraries
// Author: AI Enhanced - 2025-07-09

#include "universal_json_adapter.h"

// Additional includes for specific backends
#if JSON_ADAPTER_BACKEND == JSON11
#include <json11.hpp>
#elif JSON_ADAPTER_BACKEND == RAPIDJSON
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>
#endif

#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <shared_mutex>
#include <mutex>
#include <string>
#include <string_view>
#include <algorithm>
#include <memory>
#include <chrono>
#include <future>
#include <optional>
#include <variant>
#include <type_traits>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>

namespace universal_observable_json {

// Use the universal JSON adapter
using json = json_adapter::json;

// Thread pool for efficient async operations
class ThreadPool {
public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) 
        : stop_(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
        using return_type = typename std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_.load()) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return result;
    }
    
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};

// Simplified callback signature for all events
using CallbackFunction = std::function<void(const json&, const std::string&, const json&)>;

// Event filter for sophisticated event handling
class EventFilter {
public:
    EventFilter() = default;
    
    EventFilter& path(const std::string& p) {
        path_filter_ = p;
        return *this;
    }
    
    EventFilter& type(const std::string& t) {
        type_filter_ = t;
        return *this;
    }
    
    EventFilter& value_matches(std::function<bool(const json&)> predicate) {
        value_predicate_ = std::move(predicate);
        return *this;
    }
    
    EventFilter& debounce(std::chrono::milliseconds delay) {
        debounce_delay_ = delay;
        return *this;
    }
    
    bool matches(const std::string& path, const std::string& type, const json& value) const {
        if (path_filter_ && *path_filter_ != path) return false;
        if (type_filter_ && *type_filter_ != type) return false;
        if (value_predicate_ && !(*value_predicate_)(value)) return false;
        return true;
    }
    
    std::chrono::milliseconds get_debounce_delay() const {
        return debounce_delay_;
    }
    
private:
    std::optional<std::string> path_filter_;
    std::optional<std::string> type_filter_;
    std::optional<std::function<bool(const json&)>> value_predicate_;
    std::chrono::milliseconds debounce_delay_{0};
};

// RAII subscription handle
class SubscriptionHandle {
public:
    SubscriptionHandle() = default;
    SubscriptionHandle(std::function<void()> unsubscriber) 
        : unsubscriber_(std::move(unsubscriber)), valid_(true) {}
    
    ~SubscriptionHandle() {
        if (valid_ && unsubscriber_) {
            try {
                unsubscriber_();
            } catch (...) {
                // Don't throw from destructor
            }
        }
    }
    
    // Move-only semantics
    SubscriptionHandle(const SubscriptionHandle&) = delete;
    SubscriptionHandle& operator=(const SubscriptionHandle&) = delete;
    
    SubscriptionHandle(SubscriptionHandle&& other) noexcept 
        : unsubscriber_(std::move(other.unsubscriber_)), valid_(other.valid_) {
        other.valid_ = false;
    }
    
    SubscriptionHandle& operator=(SubscriptionHandle&& other) noexcept {
        if (this != &other) {
            if (valid_ && unsubscriber_) {
                unsubscriber_();
            }
            unsubscriber_ = std::move(other.unsubscriber_);
            valid_ = other.valid_;
            other.valid_ = false;
        }
        return *this;
    }
    
    void unsubscribe() {
        if (valid_ && unsubscriber_) {
            unsubscriber_();
            valid_ = false;
        }
    }
    
    bool is_valid() const { return valid_; }
    
private:
    std::function<void()> unsubscriber_;
    bool valid_ = false;
};

// Batch operation context
struct BatchContext {
    std::vector<std::pair<std::string, json>> changes;
    std::chrono::steady_clock::time_point start_time;
    
    BatchContext() : start_time(std::chrono::steady_clock::now()) {}
    
    void add_change(const std::string& path, const json& , const json& new_value) {
        changes.emplace_back(path, new_value);
    }
    
    size_t size() const { return changes.size(); }
    bool empty() const { return changes.empty(); }
};

// THE UNIVERSAL OBSERVABLE JSON CLASS - SIMPLIFIED VERSION
class UniversalObservableJson final {
public:
    UniversalObservableJson() = default;
    
    // Constructor from JSON string
    explicit UniversalObservableJson(const std::string& json_str) 
        : data_(json_adapter::parse(json_str)) {}
    
    // Constructor from JSON object
    explicit UniversalObservableJson(const json& initial_data) 
        : data_(initial_data) {}
    
    // Subscribe to changes (simplified version)
    size_t subscribe(CallbackFunction callback) {
        std::lock_guard<std::mutex> lock(subscribers_mutex_);
        size_t id = next_id_++;
        subscribers_[id] = callback;
        return id;
    }
    
    // Unsubscribe
    void unsubscribe(size_t id) {
        std::lock_guard<std::mutex> lock(subscribers_mutex_);
        subscribers_.erase(id);
    }
    
    // Set value (simplified - only supports root level keys for now)
    template<typename T>
    void set(const std::string& key, const T& value) {
        json old_value = json_adapter::make_null();
        
        {
            std::unique_lock<std::shared_mutex> lock(data_mutex_);
            
            // Get old value if exists
            if (json_adapter::is_object(data_) && json_adapter::has_key(data_, key)) {
                old_value = json_adapter::object_at(data_, key);
            }
            
            // Set new value - backend specific implementation
            #if JSON_ADAPTER_BACKEND == NLOHMANN_JSON
                data_[key] = value;
            #elif JSON_ADAPTER_BACKEND == JSON11
                // For json11, we need to rebuild the object
                auto obj = json_adapter::is_object(data_) ? data_.object_items() : std::map<std::string, json11::Json>();
                obj[key] = json11::Json(value);
                data_ = json11::Json(obj);
            #elif JSON_ADAPTER_BACKEND == RAPIDJSON
                // Optimized RapidJSON: Direct member update instead of remove+add
                rapidjson::Value::MemberIterator member = data_.doc.FindMember(key.c_str());
                if (member != data_.doc.MemberEnd()) {
                    // Update existing member
                    if constexpr (std::is_same_v<T, bool>) {
                        member->value.SetBool(value);
                    } else if constexpr (std::is_same_v<T, int>) {
                        member->value.SetInt(value);
                    } else if constexpr (std::is_same_v<T, double>) {
                        member->value.SetDouble(value);
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        member->value.SetString(value.c_str(), value.length(), data_.doc.GetAllocator());
                    } else if constexpr (std::is_same_v<T, const char*>) {
                        member->value.SetString(value, data_.doc.GetAllocator());
                    } else {
                        // Try to handle other numeric types
                        if constexpr (std::is_integral_v<T>) {
                            member->value.SetInt(static_cast<int>(value));
                        } else if constexpr (std::is_floating_point_v<T>) {
                            member->value.SetDouble(static_cast<double>(value));
                        } else {
                            throw std::runtime_error("Unsupported value type for RapidJSON");
                        }
                    }
                } else {
                    // Add new member
                    rapidjson::Value k(key.c_str(), data_.doc.GetAllocator());
                    rapidjson::Value v;
                    if constexpr (std::is_same_v<T, bool>) {
                        v.SetBool(value);
                    } else if constexpr (std::is_same_v<T, int>) {
                        v.SetInt(value);
                    } else if constexpr (std::is_same_v<T, double>) {
                        v.SetDouble(value);
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        v.SetString(value.c_str(), value.length(), data_.doc.GetAllocator());
                    } else if constexpr (std::is_same_v<T, const char*>) {
                        v.SetString(value, data_.doc.GetAllocator());
                    } else {
                        // Try to handle other numeric types
                        if constexpr (std::is_integral_v<T>) {
                            v.SetInt(static_cast<int>(value));
                        } else if constexpr (std::is_floating_point_v<T>) {
                            v.SetDouble(static_cast<double>(value));
                        } else {
                            throw std::runtime_error("Unsupported value type for RapidJSON");
                        }
                    }
                    data_.doc.AddMember(k, v, data_.doc.GetAllocator());
                }
            #else
                // For other backends, would need specific implementation
                throw std::runtime_error("Set operation not fully implemented for this backend");
            #endif
        }
        
        // Get new value and notify
        json new_value = json_adapter::object_at(data_, key);
        notify_subscribers(new_value, key, old_value);
    }
    
    // Get value
    template<typename T = json>
    T get(const std::string& key = "") const {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        
        if (key.empty()) {
            if constexpr (std::is_same_v<T, json>) {
                return data_;
            } else {
                return extract_value<T>(data_);
            }
        } else {
            if (json_adapter::is_object(data_) && json_adapter::has_key(data_, key)) {
                json value = json_adapter::object_at(data_, key);
                if constexpr (std::is_same_v<T, json>) {
                    return value;
                } else {
                    return extract_value<T>(value);
                }
            } else {
                throw std::runtime_error("Key not found: " + key);
            }
        }
    }
    
    // Check if key exists
    bool has(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        return json_adapter::is_object(data_) && json_adapter::has_key(data_, key);
    }
    
    // Remove key
    void remove(const std::string& key) {
        json old_value;
        
        {
            std::unique_lock<std::shared_mutex> lock(data_mutex_);
            
            if (json_adapter::is_object(data_) && json_adapter::has_key(data_, key)) {
                old_value = json_adapter::object_at(data_, key);
                
                #if JSON_ADAPTER_BACKEND == NLOHMANN_JSON
                    data_.erase(key);
                #elif JSON_ADAPTER_BACKEND == JSON11
                    // For json11, we need to rebuild the object
                    auto obj = data_.object_items();
                    obj.erase(key);
                    data_ = json11::Json(obj);
                #elif JSON_ADAPTER_BACKEND == RAPIDJSON
                    // For RapidJSON, remove member
                    data_.doc.RemoveMember(key.c_str());
                #else
                    throw std::runtime_error("Remove operation not fully implemented for this backend");
                #endif
            }
        }
        
        notify_subscribers(json_adapter::make_null(), key, old_value);
    }
    
    // Get JSON string representation
    std::string dump(int indent = -1) const {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        return json_adapter::dump(data_, indent);
    }
    
    // Get subscriber count
    size_t get_subscriber_count() const {
        std::lock_guard<std::mutex> lock(subscribers_mutex_);
        return subscribers_.size();
    }
    
    // Clear all data
    void clear() {
        json old_data;
        {
            std::unique_lock<std::shared_mutex> lock(data_mutex_);
            old_data = std::move(data_);
            data_ = json_adapter::make_object();
        }
        
        notify_subscribers(json_adapter::make_object(), "", old_data);
    }
    
private:
    mutable json data_ = json_adapter::make_object();
    mutable std::shared_mutex data_mutex_;
    std::unordered_map<size_t, CallbackFunction> subscribers_;
    mutable std::mutex subscribers_mutex_;
    size_t next_id_ = 1;
    
    template<typename T>
    T extract_value(const json& j) const {
        if constexpr (std::is_same_v<T, bool>) {
            return json_adapter::get_bool(j);
        } else if constexpr (std::is_same_v<T, int>) {
            return json_adapter::get_int(j);
        } else if constexpr (std::is_same_v<T, double>) {
            return json_adapter::get_double(j);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return json_adapter::get_string(j);
        } else {
            // For other types, try direct conversion
            return T{};
        }
    }
    
    void notify_subscribers(const json& new_value, const std::string& key, const json& old_value) {
        std::lock_guard<std::mutex> lock(subscribers_mutex_);
        for (auto& [id, callback] : subscribers_) {
            try {
                callback(new_value, key, old_value);
            } catch (const std::exception& e) {
                // Handle callback exceptions gracefully
                std::cerr << "Callback error: " << e.what() << std::endl;
            }
        }
    }
};

// Type alias for convenience
using ObservableJson = UniversalObservableJson;

} // namespace universal_observable_json
