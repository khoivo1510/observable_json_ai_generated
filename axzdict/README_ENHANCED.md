# AxzDict Enhanced - C++17 Dictionary Library

## Overview

AxzDict is a modern C++17 enhanced dictionary/JSON library that provides type-safe, thread-safe, and performance-optimized operations. This enhanced version includes significant improvements over the original implementation while maintaining full backwards compatibility.

## Key Features

### 🔒 Thread Safety
- Built-in thread-safe operations with shared_mutex
- Read-write locks for optimal performance
- Manual lock/unlock methods for complex operations

### 🛡️ Memory Safety
- Bounds checking for all array/object access
- Exception-safe operations
- RAII-compliant resource management
- Safe access methods returning std::optional

### ⚡ Performance Optimizations
- C++17 features: std::optional, std::string_view, if constexpr
- Move semantics optimization
- Reserve capacity for containers
- Efficient iterator support

### 🎯 Enhanced API
- Safe access methods with optional return types
- String view support for keys
- Range-based for loop support
- Template get_if<T>() for type-safe value extraction

## Basic Usage

### Creating and Accessing Data

```cpp
#include "axz_dict.h"
#include "axz_json.h"

// Create different types
AxzDict null_val = nullptr;
AxzDict int_val = 42;
AxzDict double_val = 3.14;
AxzDict string_val = L"Hello World";
AxzDict bool_val = true;

// Create arrays
AxzDict array = axz_dict_array{1, 2, 3, 4, 5};

// Create objects
AxzDict object = axz_dict_object{
    {L"name", L"John"},
    {L"age", 30},
    {L"active", true}
};
```

### Safe Access Methods

```cpp
// Traditional access (with bounds checking)
try {
    int value = array[2].intVal();  // Safe, throws on invalid index
    std::wcout << L"Value: " << value << std::endl;
} catch (const std::out_of_range& e) {
    std::wcout << L"Index out of range!" << std::endl;
}

// Safe access with optional return
auto safe_value = array.at_safe(2);
if (safe_value.has_value()) {
    std::wcout << L"Safe value: " << safe_value->intVal() << std::endl;
} else {
    std::wcout << L"Index not found!" << std::endl;
}

// Safe reference access
auto safe_ref = array.at_safe(2);
if (safe_ref.has_value()) {
    safe_ref->get() = 999;  // Modify in place
}
```

### String View Support

```cpp
AxzDict obj = axz_dict_object{{L"long_key_name", 123}};

// Use string view for efficiency
std::wstring_view key = L"long_key_name";
auto value = obj.at_safe(key);

// Or directly with string literals
auto value2 = obj[std::wstring_view(L"long_key_name")];
```

### Type-Safe Value Extraction

```cpp
AxzDict mixed_data = axz_dict_object{
    {L"integer", 42},
    {L"decimal", 3.14},
    {L"text", L"Hello"},
    {L"flag", true}
};

// Safe type extraction
auto int_val = mixed_data[L"integer"].get_if<int32_t>();
if (int_val.has_value()) {
    std::wcout << L"Integer: " << *int_val << std::endl;
}

auto double_val = mixed_data[L"decimal"].get_if<double>();
if (double_val.has_value()) {
    std::wcout << L"Double: " << *double_val << std::endl;
}

// Won't work - wrong type
auto wrong_type = mixed_data[L"integer"].get_if<axz_wstring>();
assert(!wrong_type.has_value());
```

### Iterator Support

```cpp
// Range-based for loops
AxzDict array = axz_dict_array{1, 2, 3, 4, 5};

std::wcout << L"Array elements: ";
for (const auto& element : array) {
    std::wcout << element.intVal() << L" ";
}
std::wcout << std::endl;

// Object iteration
AxzDict obj = axz_dict_object{{L"a", 1}, {L"b", 2}, {L"c", 3}};

std::wcout << L"Object values: ";
for (const auto& value : obj) {
    std::wcout << value.intVal() << L" ";
}
std::wcout << std::endl;

// Traditional iterators
auto it = array.begin();
while (it != array.end()) {
    std::wcout << it->intVal() << L" ";
    ++it;
}
```

### Thread-Safe Operations

```cpp
#include <thread>
#include <vector>

AxzDict shared_counter = axz_dict_object{{L"count", 0}};

// Manual locking
void increment_counter() {
    shared_counter.lock();
    int current = shared_counter[L"count"].intVal();
    shared_counter[L"count"] = current + 1;
    shared_counter.unlock();
}

// Shared (read) locking
void read_counter() {
    shared_counter.lock_shared();
    int current = shared_counter[L"count"].intVal();
    std::wcout << L"Current count: " << current << std::endl;
    shared_counter.unlock_shared();
}

// Use with multiple threads
std::vector<std::thread> threads;
for (int i = 0; i < 10; ++i) {
    threads.emplace_back(increment_counter);
}
for (auto& t : threads) {
    t.join();
}
```

### JSON Serialization

```cpp
// Create complex data structure
AxzDict data = axz_dict_object{
    {L"user", axz_dict_object{
        {L"id", 123},
        {L"name", L"John Doe"},
        {L"email", L"john@example.com"}
    }},
    {L"settings", axz_dict_object{
        {L"theme", L"dark"},
        {L"notifications", true}
    }},
    {L"scores", axz_dict_array{95, 87, 92, 88}}
};

// Serialize to JSON
axz_wstring json_string;
auto result = AxzJson::serialize(data, json_string, true);  // Pretty print
if (result == AXZ_OK) {
    std::wcout << L"JSON: " << json_string << std::endl;
}

// Deserialize from JSON
AxzDict parsed_data;
result = AxzJson::deserialize(json_string, parsed_data);
if (result == AXZ_OK) {
    std::wcout << L"Parsed user name: " << parsed_data[L"user"][L"name"].stringVal() << std::endl;
}
```

### Utility Methods

```cpp
AxzDict array = axz_dict_array{1, 2, 3};

// Check if empty
if (array.empty()) {
    std::wcout << L"Array is empty" << std::endl;
} else {
    std::wcout << L"Array has " << array.size() << L" elements" << std::endl;
}

// Reserve capacity for performance
AxzDict large_array = axz_dict_array{};
large_array.reserve(1000);  // Pre-allocate space

// Clear contents
array.clear();
assert(array.empty());

// Drop to null
array.drop();
assert(array.isNull());
```

### Error Handling

```cpp
// Exception-safe operations
AxzDict data = axz_dict_array{1, 2, 3};

try {
    // This will throw std::out_of_range
    int value = data[10].intVal();
} catch (const std::out_of_range& e) {
    std::wcout << L"Caught exception: " << e.what() << std::endl;
    // Object is still valid after exception
    assert(data.size() == 3);
}

// Safe alternative
auto safe_value = data.at_safe(10);
if (!safe_value.has_value()) {
    std::wcout << L"Index 10 not found" << std::endl;
}
```

## Advanced Features

### Custom Callable Objects

```cpp
// Create callable AxzDict
auto calculator = [](AxzDict&& input) -> AxzDict {
    if (input.isObject()) {
        int a = input[L"a"].intVal();
        int b = input[L"b"].intVal();
        return axz_dict_object{{L"result", a + b}};
    }
    return axz_dict_object{{L"error", L"Invalid input"}};
};

AxzDict callable_dict = axz_dict_callable(calculator);

// Use the callable
AxzDict input = axz_dict_object{{L"a", 5}, {L"b", 3}};
AxzDict result = callable_dict(std::move(input));
std::wcout << L"Result: " << result[L"result"].intVal() << std::endl;  // 8
```

### Dot Notation Access

```cpp
AxzDict nested = axz_dict_object{
    {L"user", axz_dict_object{
        {L"profile", axz_dict_object{
            {L"name", L"John"},
            {L"age", 30}
        }}
    }}
};

// Access nested values with dot notation
axz_wstring name;
auto rc = nested.dotVal(L"user.profile.name", name);
if (rc == AXZ_OK) {
    std::wcout << L"Name: " << name << std::endl;
}

// Check if nested path exists
rc = nested.contain(L"user.profile.email");
if (rc == AXZ_ERROR_NOT_FOUND) {
    std::wcout << L"Email not found" << std::endl;
}
```

## Performance Considerations

### Memory Management
- Uses `std::shared_ptr` for automatic memory management
- Thread-safe reference counting
- Efficient move semantics for large objects

### Threading
- Readers-writer locks for optimal concurrent access
- Shared locks for read operations
- Exclusive locks for write operations

### Optimization Tips
1. Use `reserve()` for large containers
2. Use `std::wstring_view` for key lookups
3. Use `get_if<T>()` for type-safe extraction
4. Use `at_safe()` instead of exceptions for control flow
5. Use move semantics when possible

## Migration Guide

### From Original AxzDict

The enhanced version is fully backwards compatible. Existing code will continue to work with these improvements:

1. **Bounds Checking**: `operator[]` now throws `std::out_of_range` instead of undefined behavior
2. **Thread Safety**: All operations are now thread-safe by default
3. **Exception Safety**: Strong exception safety guarantees

### Recommended Updates

```cpp
// Old code (still works)
int value = dict[L"key"].intVal();

// New recommended approach
auto value = dict.at_safe(L"key");
if (value.has_value()) {
    int result = value->intVal();
}

// Or with type safety
auto typed_value = dict[L"key"].get_if<int32_t>();
if (typed_value.has_value()) {
    int result = *typed_value;
}
```

## Building

The library requires C++17 support. Add to your CMakeLists.txt:

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Link the library
target_link_libraries(your_target axzdct)
```

## Testing

Run the enhanced test suite:

```bash
cd axzdict
g++ -std=c++17 -pthread -o test_enhanced axz_dict_enhanced_test.cpp axz_dict.cpp axz_json.cpp
./test_enhanced
```

## License

Same as the original AxzDict library.
