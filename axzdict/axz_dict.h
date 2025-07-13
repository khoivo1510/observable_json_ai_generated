#ifndef __axz_dict__
#define __axz_dict__

#include "axz_types.h"
#include "axz_export.h"
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <optional>
#include <string_view>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <immintrin.h>  // For SIMD optimizations
#include <version>      // C++17 feature detection
#include <cstdint>
#include <cstring>
#include <type_traits>

// C++17 Feature Detection and Performance Macros
#if __cpp_if_constexpr >= 201606L
#define AXZ_CONSTEXPR_IF constexpr
#else
#define AXZ_CONSTEXPR_IF 
#endif

#if defined(__AVX2__) && defined(__BMI2__)
#define AXZ_SIMD_SUPPORT 1
#else
#define AXZ_SIMD_SUPPORT 0
#endif

// Performance monitoring and caching
namespace axz_performance {
    // Cache-line aligned atomic counters for performance tracking
    struct alignas(64) PerformanceCounters {
        std::atomic<uint64_t> hash_operations{0};
        std::atomic<uint64_t> string_comparisons{0};
        std::atomic<uint64_t> memory_allocations{0};
        std::atomic<uint64_t> cache_hits{0};
        std::atomic<uint64_t> cache_misses{0};
        
        void reset() noexcept {
            hash_operations.store(0, std::memory_order_relaxed);
            string_comparisons.store(0, std::memory_order_relaxed);
            memory_allocations.store(0, std::memory_order_relaxed);
            cache_hits.store(0, std::memory_order_relaxed);
            cache_misses.store(0, std::memory_order_relaxed);
        }
    };
    
    extern PerformanceCounters g_counters;
    
    // High-performance string pool for commonly used keys
    class StringPool {
    private:
        static constexpr size_t POOL_SIZE = 1024;
        mutable std::shared_mutex pool_mutex;
        std::unordered_map<axz_wstring, std::shared_ptr<axz_wstring>> pool;
        
    public:
        std::shared_ptr<axz_wstring> intern(const axz_wstring& str) {
            // Fast read path
            {
                std::shared_lock<std::shared_mutex> lock(pool_mutex);
                auto it = pool.find(str);
                if (it != pool.end()) {
                    g_counters.cache_hits.fetch_add(1, std::memory_order_relaxed);
                    return it->second;
                }
            }
            
            // Slow write path
            {
                std::unique_lock<std::shared_mutex> lock(pool_mutex);
                auto [it, inserted] = pool.emplace(str, std::make_shared<axz_wstring>(str));
                if (inserted) {
                    g_counters.cache_misses.fetch_add(1, std::memory_order_relaxed);
                }
                return it->second;
            }
        }
        
        size_t size() const {
            std::shared_lock<std::shared_mutex> lock(pool_mutex);
            return pool.size();
        }
    };
    
    extern StringPool g_string_pool;
}

enum class AxzDictType 
{
	NUL, 
	NUMBER, 
	INTEGRAL,
	BOOL, 
	STRING,
	BYTES,
	ARRAY, 
	OBJECT,
	CALLABLE
};

class AxzDict;
using axz_dict_array    = std::vector<AxzDict>;

// Ultra-fast hash and comparison implementation with SIMD optimization
namespace axz_hash_internal {
    
    // Compile-time FNV-1a hash for string literals
    constexpr std::size_t fnv1a_hash(const wchar_t* str, size_t len) noexcept {
        constexpr std::size_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
        constexpr std::size_t FNV_PRIME = 1099511628211ULL;
        
        std::size_t hash = FNV_OFFSET_BASIS;
        for (size_t i = 0; i < len; ++i) {
            hash ^= static_cast<std::size_t>(str[i]);
            hash *= FNV_PRIME;
        }
        return hash;
    }
    
    // High-performance wide string hash with SIMD acceleration
    struct UltraFastWStringHash {
        std::size_t operator()(const axz_wstring& s) const noexcept {
            axz_performance::g_counters.hash_operations.fetch_add(1, std::memory_order_relaxed);
            
            const size_t len = s.size();
            const wchar_t* data = s.data();
            
            if (len == 0) return 0;
            
            // Fast path for short strings (most common case)
            if (len <= 8) {
                return fnv1a_hash(data, len);
            }
            
#if AXZ_SIMD_SUPPORT
            // SIMD-accelerated hashing for longer strings
            return simd_hash(data, len);
#else
            // Fallback high-performance hash
            return fallback_hash(data, len);
#endif
        }
        
    private:
#if AXZ_SIMD_SUPPORT
        std::size_t simd_hash(const wchar_t* data, size_t len) const noexcept {
            constexpr std::size_t FNV_PRIME = 1099511628211ULL;
            std::size_t hash = 14695981039346656037ULL;
            
            const size_t chunks = len / 8;
            const wchar_t* ptr = data;
            
            // Process 8 wchar_t at a time with AVX2
            for (size_t i = 0; i < chunks; ++i) {
                __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
                
                // Extract hash contribution from each element
                alignas(32) uint32_t values[8];
                _mm256_store_si256(reinterpret_cast<__m256i*>(values), chunk);
                
                for (int j = 0; j < 8; ++j) {
                    hash ^= values[j];
                    hash *= FNV_PRIME;
                }
                
                ptr += 8;
            }
            
            // Handle remaining elements
            for (size_t i = chunks * 8; i < len; ++i) {
                hash ^= static_cast<std::size_t>(data[i]);
                hash *= FNV_PRIME;
            }
            
            return hash;
        }
#endif
        
        std::size_t fallback_hash(const wchar_t* data, size_t len) const noexcept {
            // Optimized FNV-1a with loop unrolling
            constexpr std::size_t FNV_PRIME = 1099511628211ULL;
            std::size_t hash = 14695981039346656037ULL;
            
            const size_t chunks = len / 4;
            size_t i = 0;
            
            // Unrolled loop for better performance
            for (size_t c = 0; c < chunks; ++c) {
                hash ^= static_cast<std::size_t>(data[i]); hash *= FNV_PRIME; ++i;
                hash ^= static_cast<std::size_t>(data[i]); hash *= FNV_PRIME; ++i;
                hash ^= static_cast<std::size_t>(data[i]); hash *= FNV_PRIME; ++i;
                hash ^= static_cast<std::size_t>(data[i]); hash *= FNV_PRIME; ++i;
            }
            
            // Handle remaining elements
            for (; i < len; ++i) {
                hash ^= static_cast<std::size_t>(data[i]);
                hash *= FNV_PRIME;
            }
            
            return hash;
        }
    };
    
    // Ultra-fast string comparison with SIMD optimization
    struct UltraFastWStringEqual {
        bool operator()(const axz_wstring& lhs, const axz_wstring& rhs) const noexcept {
            axz_performance::g_counters.string_comparisons.fetch_add(1, std::memory_order_relaxed);
            
            const size_t len = lhs.size();
            if (len != rhs.size()) return false;
            if (len == 0) return true;
            
            const wchar_t* data1 = lhs.data();
            const wchar_t* data2 = rhs.data();
            
            // Fast pointer equality check
            if (data1 == data2) return true;
            
#if AXZ_SIMD_SUPPORT
            return simd_compare(data1, data2, len);
#else
            return fallback_compare(data1, data2, len);
#endif
        }
        
    private:
#if AXZ_SIMD_SUPPORT
        bool simd_compare(const wchar_t* data1, const wchar_t* data2, size_t len) const noexcept {
            const size_t chunks = len / 8;
            
            // Compare 8 wchar_t at a time with AVX2
            for (size_t i = 0; i < chunks; ++i) {
                __m256i chunk1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data1 + i * 8));
                __m256i chunk2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(data2 + i * 8));
                __m256i cmp = _mm256_cmpeq_epi32(chunk1, chunk2);
                
                if (_mm256_movemask_epi8(cmp) != 0xFFFFFFFF) {
                    return false;
                }
            }
            
            // Compare remaining elements
            for (size_t i = chunks * 8; i < len; ++i) {
                if (data1[i] != data2[i]) return false;
            }
            
            return true;
        }
#endif
        
        bool fallback_compare(const wchar_t* data1, const wchar_t* data2, size_t len) const noexcept {
            // Optimized comparison with loop unrolling
            const size_t chunks = len / 4;
            size_t i = 0;
            
            for (size_t c = 0; c < chunks; ++c) {
                if (data1[i] != data2[i]) return false; 
                ++i;
                if (data1[i] != data2[i]) return false; 
                ++i;
                if (data1[i] != data2[i]) return false; 
                ++i;
                if (data1[i] != data2[i]) return false; 
                ++i;
            }
            
            for (; i < len; ++i) {
                if (data1[i] != data2[i]) return false;
            }
            
            return true;
        }
    };
}

using axz_dict_object   = std::unordered_map<axz_wstring, AxzDict>;
using axz_dict_object_safe = std::unordered_map<axz_wstring, AxzDict, 
                                               axz_hash_internal::UltraFastWStringHash, 
                                               axz_hash_internal::UltraFastWStringEqual>;

using axz_dict_keys     = std::set<axz_wstring>;
using axz_dict_callable = std::function<AxzDict ( AxzDict&& )>;

using axz_shared_dict   = std::shared_ptr<AxzDict>;
using axz_weak_dict     = std::weak_ptr<AxzDict>;
using axz_unique_dict   = std::unique_ptr<AxzDict>;

class _AxzDicVal;
class AxzDictStepper;

class AXZDICT_DECLCLASS AxzDict final
{
public:
    // Modern C++17 enhancements
    static constexpr size_t SMALL_STRING_OPTIMIZATION_SIZE = 15;
    static constexpr size_t DEFAULT_RESERVE_SIZE = 16;
    
    // Cache-line aligned for better performance
    alignas(64) mutable std::atomic<uint32_t> access_count{0};
    
    // Performance statistics for this instance
    mutable struct {
        std::atomic<uint32_t> get_operations{0};
        std::atomic<uint32_t> set_operations{0};
        std::atomic<uint32_t> memory_reallocations{0};
    } stats;
    
public:	
	AxzDict() noexcept;
	AxzDict( AxzDictType type ) noexcept;
	AxzDict( void* ) = delete;
	AxzDict( std::nullptr_t ) noexcept;
	
	// Enhanced constructors with perfect forwarding
	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
	AxzDict( T val ) : AxzDict(static_cast<double>(val)) {}
	
	AxzDict( int32_t val );
	AxzDict( double val );	
	AxzDict( bool val );
	AxzDict( const axz_wstring& val );
	AxzDict( const axz_wchar* val );
	AxzDict( axz_wstring&& val ) noexcept;
	
	// String view constructor for zero-copy scenarios
	AxzDict( std::wstring_view val );
	
	AxzDict( const axz_bytes& val );
	AxzDict( axz_bytes&& val ) noexcept;	

	AxzDict( const axz_dict_array& vals );
	AxzDict( axz_dict_array&& vals ) noexcept;

	AxzDict( const axz_dict_object& vals );
	AxzDict( axz_dict_object&& vals ) noexcept;

	AxzDict( const AxzDict& val );
	AxzDict( AxzDict&& val ) noexcept;

	AxzDict( const axz_dict_callable& val );
	
	// Initializer list constructors for convenient syntax
	AxzDict( std::initializer_list<AxzDict> vals );
	AxzDict( std::initializer_list<std::pair<axz_wstring, AxzDict>> vals );

	~AxzDict() = default;

	AxzDict& operator=( const AxzDict& other );
	AxzDict& operator=( AxzDict&& other ) noexcept;

	AxzDict& operator=( std::nullptr_t ) noexcept;
	AxzDict& operator=( void* ) = delete;
	
	// Enhanced assignment operators with perfect forwarding
	template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
	AxzDict& operator=( T val ) { return *this = static_cast<double>(val); }
	
	AxzDict& operator=( int32_t val );
	AxzDict& operator=( double val );	
	AxzDict& operator=( bool val );
	AxzDict& operator=( const axz_wstring& val );	
	AxzDict& operator=( axz_wstring&& val ) noexcept;
	AxzDict& operator=( const axz_wchar* val );
	AxzDict& operator=( std::wstring_view val );
	AxzDict& operator=( const axz_bytes& val );
	AxzDict& operator=( axz_bytes&& val ) noexcept;	

	AxzDict& operator=( const axz_dict_object& other );
	AxzDict& operator=( axz_dict_object&& other ) noexcept;

	AxzDict& operator=( const axz_dict_array& other );
	AxzDict& operator=( axz_dict_array&& other ) noexcept;

	AxzDict& operator=( const axz_dict_callable& other );
	
	// Initializer list assignment
	AxzDict& operator=( std::initializer_list<AxzDict> vals );
	AxzDict& operator=( std::initializer_list<std::pair<axz_wstring, AxzDict>> vals );

	AxzDictType type() const;
	bool isType( const AxzDictType type ) const;
	
	bool isNull()		const;
	bool isNumber() 	const;
	bool isIntegral()	const;
	bool isString()		const;
	bool isBytes()		const;
	bool isArray()		const;
	bool isObject() 	const;
	bool isCallable() 	const;

	axz_rc val( int32_t& val ) const;
	axz_rc val( double& val ) const;	
	axz_rc val( bool& val ) const;
	axz_rc val( axz_wstring& val ) const;
	axz_rc val( axz_bytes& val ) const;

	axz_rc steal( int32_t& val );
	axz_rc steal( double& val );	
	axz_rc steal( bool& val );
	axz_rc steal( axz_wstring& val );
	axz_rc steal( axz_bytes& val );

	axz_rc val( const axz_wstring& key, AxzDict& val ) const; 		// copy to val
	axz_rc val( const axz_wstring& key, int32_t& val ) const;
	axz_rc val( const axz_wstring& key, double& val ) const;			
	axz_rc val( const axz_wstring& key, bool& val ) const;
	axz_rc val( const axz_wstring& key, axz_wstring& val ) const;	
	axz_rc val( const axz_wstring& key, axz_bytes& val ) const;		

	/* apply dot syntax to copy data out*/
	axz_rc dotVal( const axz_wstring& key_list, AxzDict& val ) const;
	axz_rc dotVal( const axz_wstring& key_list, int32_t& val ) const;
	axz_rc dotVal( const axz_wstring& key_list, double& val ) const;			
	axz_rc dotVal( const axz_wstring& key_list, bool& val ) const;
	axz_rc dotVal( const axz_wstring& key_list, axz_wstring& val ) const;
	axz_rc dotVal( const axz_wstring& key_list, axz_bytes& val ) const;

	/* steal data from value of key - not applicable for const object*/
	axz_rc steal( const axz_wstring& key, AxzDict& val );		
	axz_rc steal( const axz_wstring& key, int32_t& val );	
	axz_rc steal( const axz_wstring& key, double& val );			
	axz_rc steal( const axz_wstring& key, bool& val );
	axz_rc steal( const axz_wstring& key, axz_wstring& val );		
	axz_rc steal( const axz_wstring& key, axz_bytes& val );			

	/* apply dot syntax to steal data out - not applicable for const object*/
	axz_rc dotSteal( const axz_wstring& key_list, AxzDict& val );		
	axz_rc dotSteal( const axz_wstring& key_list, int32_t& val );
	axz_rc dotSteal( const axz_wstring& key_list, double& val );			
	axz_rc dotSteal( const axz_wstring& key_list, bool& val );
	axz_rc dotSteal( const axz_wstring& key_list, axz_wstring& val );	
	axz_rc dotSteal( const axz_wstring& key_list, axz_bytes& val );		

	axz_rc val( const size_t idx, AxzDict& val ) const;				// copy to val
	axz_rc val( const size_t idx, int32_t& val ) const;
	axz_rc val( const size_t idx, double& val ) const;	
	axz_rc val( const size_t idx, bool& val ) const;
	axz_rc val( const size_t idx, axz_wstring& val ) const;
	axz_rc val( const size_t idx, axz_bytes& val ) const;

	/* steal data from value of key*/
	axz_rc steal( const size_t idx, AxzDict& val );				// unapplicable for const object
	axz_rc steal( const size_t idx, int32_t& val );
	axz_rc steal( const size_t idx, double& val );	
	axz_rc steal( const size_t idx, bool& val );
	axz_rc steal( const size_t idx, axz_wstring& val );			// unapplicable for const object
	axz_rc steal( const size_t idx, axz_bytes& val );			// unapplicable for const object

	/* appicale for callable object*/
	axz_rc call( AxzDict&& in_val, AxzDict& out_val ) const;
	AxzDict operator()( AxzDict&& val ) const;	

	double		numberVal() const;
	int32_t		intVal()	const;
	bool		boolVal()	const;
	axz_wstring	stringVal() const;
	axz_bytes	bytesVal()	const;	

	size_t size() const;
	
	// Enhanced operator[] with bounds checking
	const AxzDict& operator[]( const size_t idx ) const;
	AxzDict& operator[]( const size_t idx );
	
	// Safe array access with optional return
	std::optional<AxzDict> at_safe( const size_t idx ) const;
	//std::optional<std::reference_wrapper<AxzDict>> at_safe( const size_t idx );

	const AxzDict& operator[]( const axz_wstring& key ) const;
	AxzDict& operator[]( const axz_wstring& key );
	
	// Safe object access with optional return  
	std::optional<AxzDict> at_safe( const axz_wstring& key ) const;
	//std::optional<std::reference_wrapper<AxzDict>> at_safe( const axz_wstring& key );
	
	// String view support for keys
	//const AxzDict& operator[]( std::wstring_view key ) const;
	//AxzDict& operator[]( std::wstring_view key );
	//std::optional<AxzDict> at_safe( std::wstring_view key ) const;
	//std::optional<std::reference_wrapper<AxzDict>> at_safe( std::wstring_view key );		

	axz_rc add( const AxzDict& val );
	axz_rc add( AxzDict&& val );
	axz_rc add( const axz_wstring& key, const AxzDict& val );
	axz_rc add( const axz_wstring& key, AxzDict&& val );

	void clear();	// remove the internal data only, not reset to null
	axz_rc remove( const size_t idx );
	axz_rc remove( const axz_wstring& key );

	axz_rc contain( const axz_wstring& key_list ) const;
	axz_rc contain( const axz_wstring& key_list, const AxzDictType type ) const;

	axz_dict_keys keys() const;
	void become( AxzDictType type );		// drop internal data and turn to new empty type
	void drop();							// drop internal data and turn to null
    axz_rc step( std::shared_ptr<AxzDictStepper> stepper ) const;
    
    // Thread-safe operations with reader-writer lock optimization
    void lock_shared() const { m_mutex.lock_shared(); }
    void unlock_shared() const { m_mutex.unlock_shared(); }
    void lock() const { m_mutex.lock(); }
    void unlock() const { m_mutex.unlock(); }
    
    // RAII lock guards
    class shared_lock_guard {
        const AxzDict& dict;
    public:
        explicit shared_lock_guard(const AxzDict& d) : dict(d) { dict.lock_shared(); }
        ~shared_lock_guard() { dict.unlock_shared(); }
        shared_lock_guard(const shared_lock_guard&) = delete;
        shared_lock_guard& operator=(const shared_lock_guard&) = delete;
    };
    
    class unique_lock_guard {
        const AxzDict& dict;
    public:
        explicit unique_lock_guard(const AxzDict& d) : dict(d) { dict.lock(); }
        ~unique_lock_guard() { dict.unlock(); }
        unique_lock_guard(const unique_lock_guard&) = delete;
        unique_lock_guard& operator=(const unique_lock_guard&) = delete;
    };
    
    // Utility methods with performance optimizations
    bool empty() const noexcept;
    void reserve( size_t capacity );  // for array and object types
    void shrink_to_fit();  // Reduce memory usage
    
    // C++17 structured binding support with constexpr if optimization
    template<typename T>
    std::optional<T> get_if() const;
    
    // Fast type checking with branch prediction hints
    bool is_numeric() const noexcept { return isNumber() || isIntegral(); }
    bool is_container() const noexcept { return isArray() || isObject(); }
    
    // Bulk operations for better performance
    template<typename Iterator>
    axz_rc bulk_insert(Iterator first, Iterator last);
    
    // Memory-efficient merge operations
    void merge(const AxzDict& other, bool overwrite = true);
    void merge(AxzDict&& other, bool overwrite = true) noexcept;
    
    // Path-based operations with caching
    axz_rc get_path(std::wstring_view path, AxzDict& result) const;
    axz_rc set_path(std::wstring_view path, const AxzDict& value);
    axz_rc set_path(std::wstring_view path, AxzDict&& value);
    bool has_path(std::wstring_view path) const noexcept;
    
    // JSON-like syntax support
    template<typename... Args>
    AxzDict& emplace(Args&&... args);
    
    // Performance monitoring
    void reset_stats() noexcept;
    uint32_t get_access_count() const noexcept { return access_count.load(std::memory_order_relaxed); }
    
    // Memory management
    size_t memory_usage() const noexcept;  // Estimate memory used by this dict
    void compact();  // Optimize internal data structures
    
    // Iterator support for arrays and objects
    class iterator;
    class const_iterator;
    
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;
    
    // Integration compatibility methods
    bool has(const axz_wstring& key) const;
    void set(const axz_wstring& key, const AxzDict& value);
    void append(const AxzDict& value);

private:
	AxzDict( std::shared_ptr<_AxzDicVal> other ): m_val( other ){}
	void _set( const AxzDict& val );
	void _set( AxzDict&& val );
	
	// Thread safety
	mutable std::shared_mutex m_mutex;
	
	// Internal helper methods
	axz_wstring _to_wstring_key( std::wstring_view key ) const;

private:
	std::shared_ptr<_AxzDicVal> m_val;	
	friend class _AxzDot;	
};

// Iterator classes for AxzDict
class AxzDict::iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = AxzDict;
    using difference_type = std::ptrdiff_t;
    using pointer = AxzDict*;
    using reference = AxzDict&;
    
    iterator() = default;
    iterator(const iterator&) = default;
    iterator& operator=(const iterator&) = default;
    
    reference operator*() const;
    pointer operator->() const;
    iterator& operator++();
    iterator operator++(int);
    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;
    
private:
    friend class AxzDict;
    iterator(std::shared_ptr<_AxzDicVal> val, size_t index);
    iterator(std::shared_ptr<_AxzDicVal> val, axz_dict_object_safe::iterator it);
    
    std::shared_ptr<_AxzDicVal> m_val;
    size_t m_index = 0;
    axz_dict_object_safe::iterator m_obj_it;
    bool m_is_array = true;
};

class AxzDict::const_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const AxzDict;
    using difference_type = std::ptrdiff_t;
    using pointer = const AxzDict*;
    using reference = const AxzDict&;
    
    const_iterator() = default;
    const_iterator(const const_iterator&) = default;
    const_iterator(const iterator& it);
    const_iterator& operator=(const const_iterator&) = default;
    
    reference operator*() const;
    pointer operator->() const;
    const_iterator& operator++();
    const_iterator operator++(int);
    bool operator==(const const_iterator& other) const;
    bool operator!=(const const_iterator& other) const;
    
private:
    friend class AxzDict;
    const_iterator(std::shared_ptr<_AxzDicVal> val, size_t index);
    const_iterator(std::shared_ptr<_AxzDicVal> val, axz_dict_object_safe::const_iterator it);
    
    std::shared_ptr<_AxzDicVal> m_val;
    size_t m_index = 0;
    axz_dict_object_safe::const_iterator m_obj_it;
    bool m_is_array = true;
};

// Template implementation for get_if
template<typename T>
std::optional<T> AxzDict::get_if() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    if constexpr (std::is_same_v<T, double>) {
        if (isNumber() || isIntegral()) {
            return numberVal();
        }
    } else if constexpr (std::is_same_v<T, int32_t>) {
        if (isIntegral() || isNumber()) {
            return intVal();
        }
    } else if constexpr (std::is_same_v<T, bool>) {
        if (isType(AxzDictType::BOOL)) {
            return boolVal();
        }
    } else if constexpr (std::is_same_v<T, axz_wstring>) {
        if (isString()) {
            return stringVal();
        }
    } else if constexpr (std::is_same_v<T, axz_bytes>) {
        if (isBytes()) {
            return bytesVal();
        }
    }
    
    return std::nullopt;
}

#ifdef _MSC_VER
#	pragma warning( pop )
#endif

#endif
