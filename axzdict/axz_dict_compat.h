#pragma once

// Temporary compatibility layer for AxzDict integration
// This provides minimal implementations for missing AxzDict methods

#include "axz_dict.h"
#include "axz_error_codes.h"

// Implementation stubs for missing AxzDict methods
namespace AxzDictCompat {
    
    // Create AxzDict with specific type
    inline AxzDict create_typed(AxzDictType type) {
        return AxzDict(type);
    }
    
    // Safe wrapper for contain method
    inline bool safe_contain(const AxzDict& dict, const axz_wstring& key) {
        try {
            return dict.has(key);
        } catch (...) {
            return false;
        }
    }
    
    // Safe wrapper for val method
    inline bool safe_val(const AxzDict& dict, const axz_wstring& key, AxzDict& result) {
        try {
            return AXZ_SUCCESS(dict.val(key, result));
        } catch (...) {
            return false;
        }
    }
    
    // Safe wrapper for array val method
    inline bool safe_val(const AxzDict& dict, size_t index, AxzDict& result) {
        try {
            return AXZ_SUCCESS(dict.val(index, result));
        } catch (...) {
            return false;
        }
    }
}
