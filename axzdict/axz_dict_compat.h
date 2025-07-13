#pragma once

// Temporary compatibility layer for AxzDict integration
// This provides minimal implementations for missing AxzDict methods with improved safety

#include "axz_dict.h"
#include "axz_error_codes.h"
#include <stdexcept>
#include <mutex>

// Implementation stubs for missing AxzDict methods with thread safety
namespace AxzDictCompat {
    
    // Thread-safe mutex for AxzDict operations
    static std::mutex axz_operation_mutex;
    
    // Create AxzDict with specific type
    inline AxzDict create_typed(AxzDictType type) {
        try {
            return AxzDict(type);
        } catch (const std::exception&) {
            return AxzDict(); // Return null dict on error
        }
    }
    
    // Safe wrapper for contain method with thread safety and bounds checking
    inline bool safe_contain(const AxzDict& dict, const axz_wstring& key) {
        if (key.empty()) {
            return false;
        }
        
        try {
            // Use lock guard to prevent race conditions in hash table operations
            std::lock_guard<std::mutex> lock(axz_operation_mutex);
            
            // Validate dict state before operation
            if (dict.type() != AxzDictType::OBJECT) {
                return false;
            }
            
            return dict.has(key);
        } catch (const std::exception&) {
            return false; // Safe fallback
        } catch (...) {
            return false; // Handle any other exceptions
        }
    }
    
    // Safe wrapper for val method with thread safety and bounds checking
    inline bool safe_val(const AxzDict& dict, const axz_wstring& key, AxzDict& result) {
        if (key.empty()) {
            return false;
        }
        
        try {
            // Use lock guard to prevent race conditions
            std::lock_guard<std::mutex> lock(axz_operation_mutex);
            
            // Validate dict state before operation
            if (dict.type() != AxzDictType::OBJECT) {
                return false;
            }
            
            return AXZ_SUCCESS(dict.val(key, result));
        } catch (const std::exception&) {
            return false; // Safe fallback
        } catch (...) {
            return false; // Handle any other exceptions
        }
    }
    
    // Safe wrapper for array val method with thread safety and bounds checking
    inline bool safe_val(const AxzDict& dict, size_t index, AxzDict& result) {
        try {
            // Use lock guard to prevent race conditions
            std::lock_guard<std::mutex> lock(axz_operation_mutex);
            
            // Validate dict state and bounds before operation
            if (dict.type() != AxzDictType::ARRAY || index >= dict.size()) {
                return false;
            }
            
            return AXZ_SUCCESS(dict.val(index, result));
        } catch (const std::exception&) {
            return false; // Safe fallback
        } catch (...) {
            return false; // Handle any other exceptions
        }
    }
}
