#pragma once

// 🌟 UNIVERSAL JSON ADAPTER SYSTEM
// Support for ALL major JSON libraries with zero code changes
// Author: AI Enhanced - 2025-07-09

#include <string>
#include <stdexcept>
#include <vector>
#include <map>
#include <memory>

// Configuration - choose your JSON library
#ifndef JSON_ADAPTER_BACKEND
#define JSON_ADAPTER_BACKEND 1  // Default: nlohmann/json
#endif

// Backend definitions - comprehensive list
#define NLOHMANN_JSON    1
#define JSON11           2  
#define RAPIDJSON        3
#define JSONCPP          4
#define AXZDICT          5
#define BOOST_JSON       6
#define SAJSON           7
#define SIMDJSON         8
#define CPPREST          9

// Include the selected JSON library
#if JSON_ADAPTER_BACKEND == NLOHMANN_JSON
    #include <nlohmann/json.hpp>
#elif JSON_ADAPTER_BACKEND == JSON11
    #include <json11.hpp>
#elif JSON_ADAPTER_BACKEND == RAPIDJSON
    #include <rapidjson/document.h>
    #include <rapidjson/stringbuffer.h>
    #include <rapidjson/writer.h>
    #include <rapidjson/prettywriter.h>
    #include <rapidjson/error/en.h>
#elif JSON_ADAPTER_BACKEND == JSONCPP
    #include <json/json.h>
#elif JSON_ADAPTER_BACKEND == AXZDICT
    #include "axz_dict.h"
    #include "axz_json.h"
    #include "axz_dict_compat.h"
    
    // Define missing constants for AxzDict types
    #define AXZ_DICT_NULL     AxzDictType::NUL
    #define AXZ_DICT_BOOL     AxzDictType::BOOL
    #define AXZ_DICT_NUMBER   AxzDictType::NUMBER
    #define AXZ_DICT_STRING   AxzDictType::STRING
    #define AXZ_DICT_ARRAY    AxzDictType::ARRAY
    #define AXZ_DICT_OBJECT   AxzDictType::OBJECT
#elif JSON_ADAPTER_BACKEND == BOOST_JSON
    #include <boost/json.hpp>
#elif JSON_ADAPTER_BACKEND == SAJSON
    #include <sajson.h>
#elif JSON_ADAPTER_BACKEND == SIMDJSON
    #include <simdjson.h>
#elif JSON_ADAPTER_BACKEND == CPPREST
    #include <cpprest/json.h>
#endif

namespace json_adapter {

// Universal JSON type based on selected backend
#if JSON_ADAPTER_BACKEND == NLOHMANN_JSON
    using json = nlohmann::json;
    
    // Parse function for nlohmann/json
    inline json parse(const std::string& json_str) {
        return json::parse(json_str);
    }
    
    // Dump function for nlohmann/json  
    inline std::string dump(const json& j, int indent = -1) {
        return j.dump(indent);
    }
    
    // Type checking functions
    inline bool is_null(const json& j) { return j.is_null(); }
    inline bool is_bool(const json& j) { return j.is_boolean(); }
    inline bool is_number(const json& j) { return j.is_number(); }
    inline bool is_string(const json& j) { return j.is_string(); }
    inline bool is_array(const json& j) { return j.is_array(); }
    inline bool is_object(const json& j) { return j.is_object(); }
    
    // Value extraction
    inline bool get_bool(const json& j) { return j.get<bool>(); }
    inline int get_int(const json& j) { return j.get<int>(); }
    inline double get_double(const json& j) { return j.get<double>(); }
    inline std::string get_string(const json& j) { return j.get<std::string>(); }
    
    // Array operations
    inline size_t array_size(const json& j) { return j.size(); }
    inline json array_at(const json& j, size_t index) { return j[index]; }
    
    // Object operations
    inline bool has_key(const json& j, const std::string& key) { return j.contains(key); }
    inline json object_at(const json& j, const std::string& key) { return j[key]; }
    
    // Construction functions
    inline json make_null() { return json(nullptr); }
    inline json make_bool(bool value) { return json(value); }
    inline json make_int(int value) { return json(value); }
    inline json make_double(double value) { return json(value); }
    inline json make_string(const std::string& value) { return json(value); }
    inline json make_array() { return json::array(); }
    inline json make_object() { return json::object(); }
    
    // Key manipulation functions
    inline void set_member(json& obj, const std::string& key, const json& value) {
        obj[key] = value;
    }
    
    inline void remove_member(json& obj, const std::string& key) {
        obj.erase(key);
    }
    
    // Array manipulation functions
    inline void append_array(json& arr, const json& value) {
        arr.push_back(value);
    }
    
    inline void clear_array(json& arr) {
        arr.clear();
    }
    
#elif JSON_ADAPTER_BACKEND == JSON11
    using json = json11::Json;
    
    // Parse function for json11
    inline json parse(const std::string& json_str) {
        std::string err;
        auto result = json::parse(json_str, err);
        if (!err.empty()) {
            throw std::runtime_error("JSON parse error: " + err);
        }
        return result;
    }
    
    // Dump function for json11
    inline std::string dump(const json& j, int indent = -1) {
        // json11 doesn't support pretty printing, so we ignore indent
        (void)indent; // Suppress unused parameter warning
        return j.dump();
    }
    
    // Type checking functions
    inline bool is_null(const json& j) { return j.is_null(); }
    inline bool is_bool(const json& j) { return j.is_bool(); }
    inline bool is_number(const json& j) { return j.is_number(); }
    inline bool is_string(const json& j) { return j.is_string(); }
    inline bool is_array(const json& j) { return j.is_array(); }
    inline bool is_object(const json& j) { return j.is_object(); }
    
    // Value extraction
    inline bool get_bool(const json& j) { return j.bool_value(); }
    inline int get_int(const json& j) { return static_cast<int>(j.number_value()); }
    inline double get_double(const json& j) { return j.number_value(); }
    inline std::string get_string(const json& j) { return j.string_value(); }
    
    // Array operations
    inline size_t array_size(const json& j) { return j.array_items().size(); }
    inline json array_at(const json& j, size_t index) { 
        auto items = j.array_items();
        if (index >= items.size()) throw std::out_of_range("Array index out of bounds");
        return items[index];
    }
    
    // Object operations
    inline bool has_key(const json& j, const std::string& key) { 
        auto obj = j.object_items();
        return obj.find(key) != obj.end();
    }
    inline json object_at(const json& j, const std::string& key) { 
        auto obj = j.object_items();
        auto it = obj.find(key);
        if (it == obj.end()) throw std::out_of_range("Key not found: " + key);
        return it->second;
    }
    
    // Construction functions
    inline json make_null() { return json(); }
    inline json make_bool(bool value) { return json(value); }
    inline json make_int(int value) { return json(value); }
    inline json make_double(double value) { return json(value); }
    inline json make_string(const std::string& value) { return json(value); }
    inline json make_array() { return json::array(); }
    inline json make_object() { return json::object(); }
    
    // Key manipulation functions
    inline void set_member(json& obj, const std::string& key, const json& value) {
        auto obj_items = obj.object_items();
        obj_items[key] = value;
        obj = json(obj_items);
    }
    
    inline void remove_member(json& obj, const std::string& key) {
        auto obj_items = obj.object_items();
        obj_items.erase(key);
        obj = json(obj_items);
    }
    
    // Array manipulation functions
    inline void append_array(json& arr, const json& value) {
        auto arr_items = arr.array_items();
        arr_items.push_back(value);
        arr = json(arr_items);
    }
    
    inline void clear_array(json& arr) {
        arr = json::array();
    }
    
#elif JSON_ADAPTER_BACKEND == RAPIDJSON
    // RapidJSON wrapper class for universal interface
    class RapidJsonWrapper {
    public:
        rapidjson::Document doc;
        
        RapidJsonWrapper() { doc.SetObject(); }
        RapidJsonWrapper(const rapidjson::Value& val, rapidjson::Document::AllocatorType& alloc) { 
            doc.CopyFrom(val, alloc); 
        }
        
        // Copy constructor
        RapidJsonWrapper(const RapidJsonWrapper& other) {
            doc.CopyFrom(other.doc, doc.GetAllocator());
        }
        
        // Assignment operator
        RapidJsonWrapper& operator=(const RapidJsonWrapper& other) {
            if (this != &other) {
                doc.CopyFrom(other.doc, doc.GetAllocator());
            }
            return *this;
        }
        
        bool is_null() const { return doc.IsNull(); }
        bool is_bool() const { return doc.IsBool(); }
        bool is_number() const { return doc.IsNumber(); }
        bool is_string() const { return doc.IsString(); }
        bool is_array() const { return doc.IsArray(); }
        bool is_object() const { return doc.IsObject(); }
        
        bool get_bool() const { return doc.GetBool(); }
        int get_int() const { return doc.GetInt(); }
        double get_double() const { return doc.GetDouble(); }
        std::string get_string() const { return doc.GetString(); }
        
        size_t array_size() const { return doc.Size(); }
        bool has_key(const std::string& key) const { return doc.HasMember(key.c_str()); }
        
        std::string dump(int indent = -1) const {
            rapidjson::StringBuffer buffer;
            if (indent >= 0) {
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
                writer.SetIndent(' ', indent);
                doc.Accept(writer);
            } else {
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                doc.Accept(writer);
            }
            return buffer.GetString();
        }
        
        // Helper methods for object manipulation
        void set_member(const std::string& key, const rapidjson::Value& value) {
            rapidjson::Value k(key.c_str(), doc.GetAllocator());
            rapidjson::Value v;
            v.CopyFrom(value, doc.GetAllocator());
            doc.AddMember(k, v, doc.GetAllocator());
        }
        
        void remove_member(const std::string& key) {
            doc.RemoveMember(key.c_str());
        }
        
        const rapidjson::Value& get_member(const std::string& key) const {
            return doc[key.c_str()];
        }
    };
    
    using json = RapidJsonWrapper;
    
    // Parse function for RapidJSON
    inline json parse(const std::string& json_str) {
        json j;
        if (j.doc.Parse(json_str.c_str()).HasParseError()) {
            throw std::runtime_error("JSON parse error: " + 
                std::string(rapidjson::GetParseError_En(j.doc.GetParseError())));
        }
        return j;
    }
    
    // Dump function for RapidJSON
    inline std::string dump(const json& j, int indent = -1) {
        return j.dump(indent);
    }
    
    // Type checking functions
    inline bool is_null(const json& j) { return j.is_null(); }
    inline bool is_bool(const json& j) { return j.is_bool(); }
    inline bool is_number(const json& j) { return j.is_number(); }
    inline bool is_string(const json& j) { return j.is_string(); }
    inline bool is_array(const json& j) { return j.is_array(); }
    inline bool is_object(const json& j) { return j.is_object(); }
    
    // Value extraction
    inline bool get_bool(const json& j) { return j.get_bool(); }
    inline int get_int(const json& j) { return j.get_int(); }
    inline double get_double(const json& j) { return j.get_double(); }
    inline std::string get_string(const json& j) { return j.get_string(); }
    
    // Array operations
    inline size_t array_size(const json& j) { return j.array_size(); }
    inline json array_at(const json& j, size_t index) { 
        if (index >= j.array_size()) throw std::out_of_range("Array index out of bounds");
        json result;
        result.doc.CopyFrom(j.doc[index], result.doc.GetAllocator());
        return result;
    }
    
    // Object operations
    inline bool has_key(const json& j, const std::string& key) { return j.has_key(key); }
    inline json object_at(const json& j, const std::string& key) { 
        if (!j.has_key(key)) throw std::out_of_range("Key not found: " + key);
        json result;
        result.doc.CopyFrom(j.get_member(key), result.doc.GetAllocator());
        return result;
    }
    
    // Construction functions
    inline json make_null() { json j; j.doc.SetNull(); return j; }
    inline json make_bool(bool value) { json j; j.doc.SetBool(value); return j; }
    inline json make_int(int value) { json j; j.doc.SetInt(value); return j; }
    inline json make_double(double value) { json j; j.doc.SetDouble(value); return j; }
    inline json make_string(const std::string& value) { 
        json j; 
        j.doc.SetString(value.c_str(), value.length(), j.doc.GetAllocator()); 
        return j; 
    }
    inline json make_array() { json j; j.doc.SetArray(); return j; }
    inline json make_object() { json j; j.doc.SetObject(); return j; }
    
    // Key manipulation functions
    inline void set_member(json& obj, const std::string& key, const json& value) {
        obj.set_member(key, value.doc);
    }
    
    inline void remove_member(json& obj, const std::string& key) {
        obj.remove_member(key);
    }
    
    // Array manipulation functions
    inline void append_array(json& arr, const json& value) {
        rapidjson::Value v;
        v.CopyFrom(value.doc, arr.doc.GetAllocator());
        arr.doc.PushBack(v, arr.doc.GetAllocator());
    }
    
    inline void clear_array(json& arr) {
        arr.doc.SetArray();
    }
    
#elif JSON_ADAPTER_BACKEND == JSONCPP
    using json = Json::Value;
    
    // Parse function for JsonCpp
    inline json parse(const std::string& json_str) {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json_str, root)) {
            throw std::runtime_error("JSON parse error: " + reader.getFormattedErrorMessages());
        }
        return root;
    }
    
    // Dump function for JsonCpp
    inline std::string dump(const json& j, int indent = -1) {
        if (indent >= 0) {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = std::string(indent, ' ');
            return Json::writeString(builder, j);
        } else {
            Json::FastWriter writer;
            return writer.write(j);
        }
    }
    
    // Type checking functions
    inline bool is_null(const json& j) { return j.isNull(); }
    inline bool is_bool(const json& j) { return j.isBool(); }
    inline bool is_number(const json& j) { return j.isNumeric(); }
    inline bool is_string(const json& j) { return j.isString(); }
    inline bool is_array(const json& j) { return j.isArray(); }
    inline bool is_object(const json& j) { return j.isObject(); }
    
    // Value extraction
    inline bool get_bool(const json& j) { return j.asBool(); }
    inline int get_int(const json& j) { return j.asInt(); }
    inline double get_double(const json& j) { return j.asDouble(); }
    inline std::string get_string(const json& j) { return j.asString(); }
    
    // Array operations
    inline size_t array_size(const json& j) { return j.size(); }
    inline json array_at(const json& j, size_t index) { 
        if (index >= j.size()) throw std::out_of_range("Array index out of bounds");
        return j[static_cast<int>(index)];
    }
    
    // Object operations
    inline bool has_key(const json& j, const std::string& key) { return j.isMember(key); }
    inline json object_at(const json& j, const std::string& key) { 
        if (!j.isMember(key)) throw std::out_of_range("Key not found: " + key);
        return j[key];
    }
    
    // Construction functions
    inline json make_null() { return Json::Value(); }
    inline json make_bool(bool value) { return Json::Value(value); }
    inline json make_int(int value) { return Json::Value(value); }
    inline json make_double(double value) { return Json::Value(value); }
    inline json make_string(const std::string& value) { return Json::Value(value); }
    inline json make_array() { return Json::Value(Json::arrayValue); }
    inline json make_object() { return Json::Value(Json::objectValue); }
    
    // Key manipulation functions
    inline void set_member(json& obj, const std::string& key, const json& value) {
        obj[key] = value;
    }
    
    inline void remove_member(json& obj, const std::string& key) {
        obj.removeMember(key);
    }
    
    // Array manipulation functions
    inline void append_array(json& arr, const json& value) {
        arr.append(value);
    }
    
    inline void clear_array(json& arr) {
        arr.clear();
    }
    
#elif JSON_ADAPTER_BACKEND == AXZDICT
    using json = AxzDict;
    
    // Helper functions for string conversion
    inline axz_wstring to_axz_wstring(const std::string& str) {
        axz_wstring result;
        result.reserve(str.size());
        for (char c : str) {
            result.push_back(static_cast<wchar_t>(c));
        }
        return result;
    }
    
    inline std::string from_axz_wstring(const axz_wstring& wstr) {
        std::string result;
        result.reserve(wstr.size());
        for (wchar_t wc : wstr) {
            result.push_back(static_cast<char>(wc));
        }
        return result;
    }
    
    // Optimized parse function for AxzDict with thread-local caching
    inline json parse(const std::string& json_str) {
        static thread_local AxzDict cached_dict = AxzDictCompat::create_typed(AXZ_DICT_OBJECT);
        
        // Reuse cached dict for better performance
        cached_dict.clear();
        auto wstr = to_axz_wstring(json_str);
        
        if (AXZ_SUCCESS(AxzJson::deserialize(wstr, cached_dict))) {
            return cached_dict;
        } else {
            throw std::runtime_error("AxzDict JSON parse error - invalid JSON format");
        }
    }
    
    // Enhanced dump function for AxzDict with pretty printing support
    inline std::string dump(const json& j, int indent = -1) {
        static thread_local axz_wstring cached_result;
        cached_result.clear();
        
        bool pretty_format = (indent >= 0);
        
        if (AXZ_SUCCESS(AxzJson::serialize(j, cached_result, pretty_format))) {
            return from_axz_wstring(cached_result);
        } else {
            return "{}";
        }
    }
    
    // Type checking functions
    inline bool is_null(const json& j) { return j.type() == AXZ_DICT_NULL; }
    inline bool is_bool(const json& j) { return j.type() == AXZ_DICT_BOOL; }
    inline bool is_number(const json& j) { return j.type() == AXZ_DICT_NUMBER; }
    inline bool is_string(const json& j) { return j.type() == AXZ_DICT_STRING; }
    inline bool is_array(const json& j) { return j.type() == AXZ_DICT_ARRAY; }
    inline bool is_object(const json& j) { return j.type() == AXZ_DICT_OBJECT; }
    
    // Value extraction
    inline bool get_bool(const json& j) { 
        bool result = false;
        j.val(result);
        return result;
    }
    inline int get_int(const json& j) { 
        int result = 0;
        j.val(result);
        return result;
    }
    inline double get_double(const json& j) { 
        double result = 0.0;
        j.val(result);
        return result;
    }
    inline std::string get_string(const json& j) { 
        axz_wstring result;
        j.val(result);
        return from_axz_wstring(result);
    }
    
    // Array operations
    inline size_t array_size(const json& j) { return j.size(); }
    inline json array_at(const json& j, size_t index) { 
        AxzDict result;
        if (AxzDictCompat::safe_val(j, index, result)) {
            return result;
        }
        return AxzDictCompat::create_typed(AXZ_DICT_NULL);
    }
    
    // Optimized Object operations with caching
    inline bool has_key(const json& j, const std::string& key) { 
        static thread_local axz_wstring cached_key;
        cached_key = to_axz_wstring(key);
        return AxzDictCompat::safe_contain(j, cached_key);
    }
    inline json object_at(const json& j, const std::string& key) { 
        static thread_local axz_wstring cached_key;
        static thread_local AxzDict cached_result;
        
        cached_key = to_axz_wstring(key);
        if (AxzDictCompat::safe_val(j, cached_key, cached_result)) {
            return cached_result;
        }
        throw std::out_of_range("AxzDict key not found: " + key);
    }
    
    // Optimized construction functions with caching
    inline json make_null() { 
        static thread_local AxzDict cached_null = AxzDictCompat::create_typed(AXZ_DICT_NULL);
        return cached_null; 
    }
    inline json make_bool(bool value) { 
        static thread_local AxzDict cached_bool = AxzDictCompat::create_typed(AXZ_DICT_BOOL);
        cached_bool = value;
        return cached_bool;
    }
    inline json make_int(int value) { 
        static thread_local AxzDict cached_int = AxzDictCompat::create_typed(AXZ_DICT_NUMBER);
        cached_int = static_cast<int32_t>(value);
        return cached_int;
    }
    inline json make_double(double value) { 
        static thread_local AxzDict cached_double = AxzDictCompat::create_typed(AXZ_DICT_NUMBER);
        cached_double = value;
        return cached_double;
    }
    inline json make_string(const std::string& value) { 
        static thread_local AxzDict cached_string = AxzDictCompat::create_typed(AXZ_DICT_STRING);
        cached_string = to_axz_wstring(value);
        return cached_string;
    }
    inline json make_array() { 
        return AxzDictCompat::create_typed(AXZ_DICT_ARRAY); 
    }
    inline json make_object() { 
        return AxzDictCompat::create_typed(AXZ_DICT_OBJECT); 
    }
    
    // Optimized key manipulation functions
    inline void set_member(json& obj, const std::string& key, const json& value) {
        static thread_local axz_wstring cached_key;
        cached_key = to_axz_wstring(key);
        obj.set(cached_key, value);
    }
    
    inline void remove_member(json& obj, const std::string& key) {
        static thread_local axz_wstring cached_key;
        cached_key = to_axz_wstring(key);
        obj.remove(cached_key);
    }
    
    // Array manipulation functions
    inline void append_array(json& arr, const json& value) {
        arr.append(value);
    }
    
    inline void clear_array(json& arr) {
        arr.clear();
    }

#elif JSON_ADAPTER_BACKEND == BOOST_JSON
    using json = boost::json::value;
    
    // Parse function for Boost.JSON
    inline json parse(const std::string& json_str) {
        boost::json::error_code ec;
        auto result = boost::json::parse(json_str, ec);
        if (ec) {
            throw std::runtime_error("JSON parse error: " + ec.message());
        }
        return result;
    }
    
    // Dump function for Boost.JSON
    inline std::string dump(const json& j, int indent = -1) {
        if (indent >= 0) {
            return serialize(j);  // Boost.JSON doesn't have built-in pretty printing
        } else {
            return serialize(j);
        }
    }
    
    // Type checking functions
    inline bool is_null(const json& j) { return j.is_null(); }
    inline bool is_bool(const json& j) { return j.is_bool(); }
    inline bool is_number(const json& j) { return j.is_number(); }
    inline bool is_string(const json& j) { return j.is_string(); }
    inline bool is_array(const json& j) { return j.is_array(); }
    inline bool is_object(const json& j) { return j.is_object(); }
    
    // Value extraction
    inline bool get_bool(const json& j) { return j.as_bool(); }
    inline int get_int(const json& j) { return static_cast<int>(j.as_int64()); }
    inline double get_double(const json& j) { return j.as_double(); }
    inline std::string get_string(const json& j) { return std::string(j.as_string()); }
    
    // Array operations
    inline size_t array_size(const json& j) { return j.as_array().size(); }
    inline json array_at(const json& j, size_t index) { 
        auto& arr = j.as_array();
        if (index >= arr.size()) throw std::out_of_range("Array index out of bounds");
        return arr[index];
    }
    
    // Object operations
    inline bool has_key(const json& j, const std::string& key) { 
        return j.as_object().contains(key);
    }
    inline json object_at(const json& j, const std::string& key) { 
        auto& obj = j.as_object();
        auto it = obj.find(key);
        if (it == obj.end()) throw std::out_of_range("Key not found: " + key);
        return it->value();
    }
    
    // Construction functions
    inline json make_null() { return boost::json::value(); }
    inline json make_bool(bool value) { return boost::json::value(value); }
    inline json make_int(int value) { return boost::json::value(value); }
    inline json make_double(double value) { return boost::json::value(value); }
    inline json make_string(const std::string& value) { return boost::json::value(value); }
    inline json make_array() { return boost::json::array(); }
    inline json make_object() { return boost::json::object(); }
    
    // Key manipulation functions
    inline void set_member(json& obj, const std::string& key, const json& value) {
        obj.as_object()[key] = value;
    }
    
    inline void remove_member(json& obj, const std::string& key) {
        obj.as_object().erase(key);
    }
    
    // Array manipulation functions
    inline void append_array(json& arr, const json& value) {
        arr.as_array().push_back(value);
    }
    
    inline void clear_array(json& arr) {
        arr.as_array().clear();
    }
    
#elif JSON_ADAPTER_BACKEND == SAJSON
    // Note: sajson is primarily a parser, not a full JSON library
    // This would need a more complex wrapper
    #error "sajson backend not fully implemented - it's primarily a parser"
    
#elif JSON_ADAPTER_BACKEND == SIMDJSON
    // Note: simdjson is primarily a fast parser, not a full JSON library
    // This would need a more complex wrapper
    #error "simdjson backend not fully implemented - it's primarily a fast parser"
    
#elif JSON_ADAPTER_BACKEND == CPPREST
    using json = web::json::value;
    
    // Parse function for C++ REST SDK
    inline json parse(const std::string& json_str) {
        std::stringstream ss(json_str);
        web::json::value result;
        ss >> result;
        return result;
    }
    
    // Dump function for C++ REST SDK
    inline std::string dump(const json& j, int indent = -1) {
        std::stringstream ss;
        j.serialize(ss);
        return ss.str();
    }
    
    // Type checking functions
    inline bool is_null(const json& j) { return j.is_null(); }
    inline bool is_bool(const json& j) { return j.is_boolean(); }
    inline bool is_number(const json& j) { return j.is_number(); }
    inline bool is_string(const json& j) { return j.is_string(); }
    inline bool is_array(const json& j) { return j.is_array(); }
    inline bool is_object(const json& j) { return j.is_object(); }
    
    // Value extraction
    inline bool get_bool(const json& j) { return j.as_bool(); }
    inline int get_int(const json& j) { return j.as_integer(); }
    inline double get_double(const json& j) { return j.as_double(); }
    inline std::string get_string(const json& j) { return j.as_string(); }
    
    // Array operations
    inline size_t array_size(const json& j) { return j.as_array().size(); }
    inline json array_at(const json& j, size_t index) { 
        auto& arr = j.as_array();
        if (index >= arr.size()) throw std::out_of_range("Array index out of bounds");
        return arr[index];
    }
    
    // Object operations
    inline bool has_key(const json& j, const std::string& key) { 
        return j.as_object().find(utility::conversions::to_string_t(key)) != j.as_object().end();
    }
    inline json object_at(const json& j, const std::string& key) { 
        auto& obj = j.as_object();
        auto it = obj.find(utility::conversions::to_string_t(key));
        if (it == obj.end()) throw std::out_of_range("Key not found: " + key);
        return it->second;
    }
    
    // Construction functions
    inline json make_null() { return web::json::value::null(); }
    inline json make_bool(bool value) { return web::json::value::boolean(value); }
    inline json make_int(int value) { return web::json::value::number(value); }
    inline json make_double(double value) { return web::json::value::number(value); }
    inline json make_string(const std::string& value) { return web::json::value::string(utility::conversions::to_string_t(value)); }
    inline json make_array() { return web::json::value::array(); }
    inline json make_object() { return web::json::value::object(); }
    
    // Key manipulation functions
    inline void set_member(json& obj, const std::string& key, const json& value) {
        obj.as_object()[utility::conversions::to_string_t(key)] = value;
    }
    
    inline void remove_member(json& obj, const std::string& key) {
        obj.as_object().erase(utility::conversions::to_string_t(key));
    }
    
    // Array manipulation functions
    inline void append_array(json& arr, const json& value) {
        arr.as_array().push_back(value);
    }
    
    inline void clear_array(json& arr) {
        arr.as_array().clear();
    }
    
#else
    #error "Unknown JSON backend selected. Please choose from: NLOHMANN_JSON, JSON11, RAPIDJSON, JSONCPP, BOOST_JSON, SAJSON, SIMDJSON, CPPREST"
#endif

// Universal convenience functions
inline json from_string(const std::string& json_str) {
    return parse(json_str);
}

inline std::string to_string(const json& j, int indent = -1) {
    return dump(j, indent);
}

// Backend information
inline std::string get_backend_name() {
#if JSON_ADAPTER_BACKEND == NLOHMANN_JSON
    return "nlohmann/json";
#elif JSON_ADAPTER_BACKEND == JSON11
    return "json11";
#elif JSON_ADAPTER_BACKEND == RAPIDJSON
    return "RapidJSON";
#elif JSON_ADAPTER_BACKEND == JSONCPP
    return "JsonCpp";
#elif JSON_ADAPTER_BACKEND == AXZDICT
    return "AxzDict";
#elif JSON_ADAPTER_BACKEND == BOOST_JSON
    return "Boost.JSON";
#elif JSON_ADAPTER_BACKEND == SAJSON
    return "sajson";
#elif JSON_ADAPTER_BACKEND == SIMDJSON
    return "simdjson";
#elif JSON_ADAPTER_BACKEND == CPPREST
    return "cpprest";
#else
    return "Unknown";
#endif
}

inline std::string get_backend_description() {
#if JSON_ADAPTER_BACKEND == NLOHMANN_JSON
    return "Full-featured, popular JSON library with excellent C++ integration";
#elif JSON_ADAPTER_BACKEND == JSON11
    return "Lightweight, minimal dependencies, simple API";
#elif JSON_ADAPTER_BACKEND == RAPIDJSON
    return "Fast JSON parser/generator with SAX/DOM style API";
#elif JSON_ADAPTER_BACKEND == JSONCPP
    return "Mature, stable JSON library with good documentation";
#elif JSON_ADAPTER_BACKEND == AXZDICT
    return "AxzDict - Advanced dictionary-based JSON implementation";
#elif JSON_ADAPTER_BACKEND == BOOST_JSON
    return "Part of Boost libraries, header-only, fast";
#elif JSON_ADAPTER_BACKEND == SAJSON
    return "Single-header, extremely fast JSON parser";
#elif JSON_ADAPTER_BACKEND == SIMDJSON
    return "SIMD-optimized JSON parser, fastest available";
#elif JSON_ADAPTER_BACKEND == CPPREST
    return "Microsoft's C++ REST SDK JSON implementation";
#else
    return "Unknown backend";
#endif
}

} // namespace json_adapter

// Export the json type to global namespace for compatibility
using json = json_adapter::json;
