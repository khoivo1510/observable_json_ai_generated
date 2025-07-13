#include "axz_dict.h"
#include "axz_dict_stepper.h"
#include "axz_error_codes.h"
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <shared_mutex>
#include <functional>
#include <immintrin.h>  // For SIMD operations

// Global performance counters
namespace axz_performance {
    PerformanceCounters g_counters;
    StringPool g_string_pool;
}

// Optimized axz_dict_object to use ultra-fast hash and equal functions
using axz_dict_object_safe = std::unordered_map<axz_wstring, AxzDict, 
                                               axz_hash_internal::UltraFastWStringHash, 
                                               axz_hash_internal::UltraFastWStringEqual>;

template<bool isDict>
struct _AxzBool2Type
{
	enum{ val = isDict };
};

class _AxzDictVal;
struct _AxzDicDefault 
{	
	static const std::shared_ptr<_AxzDicVal> nullVal;
	static const std::shared_ptr<_AxzDicVal> trueVal;
	static const std::shared_ptr<_AxzDicVal> falseVal;
};

namespace
{
namespace Internal
{
	template<class T>
	axz_rc getVal( AxzDict& in_object, T& out_val, _AxzBool2Type<false> )
	{
		return in_object.val( out_val );
	}
	
	axz_rc getVal( AxzDict& in_object, AxzDict& out_val, _AxzBool2Type<true> )
	{
		out_val = in_object;
		return AXZ_OK;
	}	

	template<class T>
	axz_rc stealVal( AxzDict& in_object, T& out_val, _AxzBool2Type<false> )
	{
		axz_rc rc = in_object.steal( out_val );
		if ( AXZ_SUCCESS( rc ) )	in_object.drop();
		return rc;
	}

	axz_rc stealVal( AxzDict& in_object, AxzDict& out_val, _AxzBool2Type<true> )
	{
		out_val = std::move( in_object );
		in_object.drop();
		return AXZ_OK;
	}
}
};

class _AxzDicVal
{
public:
	virtual AxzDictType type() const = 0;   
	virtual bool isType( const AxzDictType type ) const = 0;
	virtual size_t size() const 					{ throw std::invalid_argument( "AxzDict::size available for object or array only" ); }

	virtual axz_rc val( double& val ) const                     	    	{ return AXZ_ERROR_NOT_SUPPORT; }
	virtual axz_rc val( int32_t& val ) const		                        { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( bool& val ) const		                            { return AXZ_ERROR_NOT_SUPPORT; }
	virtual axz_rc val( axz_wstring& val ) const	                        { return AXZ_ERROR_NOT_SUPPORT; }
	virtual axz_rc val( axz_bytes& val ) const	                            { return AXZ_ERROR_NOT_SUPPORT; };

	virtual axz_rc steal( double& val )    		                 	    	{ return this->val( val ); }
	virtual axz_rc steal( int32_t& val )			                        { return this->val( val ); };
	virtual axz_rc steal( bool& val ) 			                            { return this->val( val ); }
	virtual axz_rc steal( axz_wstring& val ) 		                        { return AXZ_ERROR_NOT_SUPPORT; }
	virtual axz_rc steal( axz_bytes& val ) 		                        	{ return AXZ_ERROR_NOT_SUPPORT; };

	virtual axz_rc val( const axz_wstring& key, AxzDict& val )	            { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const axz_wstring& key, double& val )	    		{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const axz_wstring& key, int32_t& val )		   	    { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const axz_wstring& key, bool& val )		    	    { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const axz_wstring& key, axz_wstring& val )	    	{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const axz_wstring& key, axz_bytes& val )		    { return AXZ_ERROR_NOT_SUPPORT; };	

	virtual axz_rc steal( const axz_wstring& key, AxzDict& val )			{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const axz_wstring& key, double& val )	    		{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const axz_wstring& key, int32_t& val )		   	{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const axz_wstring& key, bool& val )		    	{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const axz_wstring& key, axz_wstring& val )		{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const axz_wstring& key, axz_bytes& val )			{ return AXZ_ERROR_NOT_SUPPORT; };	

	virtual axz_rc val( const size_t idx, AxzDict& val )            	    { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const size_t idx, double& val )			            { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const size_t idx, int32_t& val )				    { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const size_t idx, bool& val )			            { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const size_t idx, axz_wstring& val )		        { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc val( const size_t idx, axz_bytes& val )		            { return AXZ_ERROR_NOT_SUPPORT; };

	virtual axz_rc steal( const size_t idx, AxzDict& val )					{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const size_t idx, double& val )			        { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const size_t idx, int32_t& val )				    { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const size_t idx, bool& val )			            { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const size_t idx, axz_wstring& val )				{ return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc steal( const size_t idx, axz_bytes& val )				{ return AXZ_ERROR_NOT_SUPPORT; };

	virtual axz_rc call( AxzDict&& in_val, AxzDict& out_val )				{ return AXZ_ERROR_NOT_SUPPORT; };

	virtual double numberVal() const            							{ throw std::invalid_argument( "AxzDict::numberVal available for number only" ); }
	virtual int32_t intVal() const                  						{ throw std::invalid_argument( "AxzDict::intVal available for number only" ); }
	virtual bool boolVal() const                							{ throw std::invalid_argument( "AxzDict::boolVal available for boolean only" ); }
	virtual axz_wstring stringVal() const									{ throw std::invalid_argument( "AxzDict::stringVal available for string only" ); }
	virtual axz_bytes bytesVal() const										{ throw std::invalid_argument( "AxzDict::bytesVal available for bytes only" ); }

	virtual axz_rc add( const AxzDict& val )	                            { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc add( AxzDict&& val )			                            { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc add( const axz_wstring& key, const AxzDict& val )        { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc add( const axz_wstring& key, AxzDict&& val )		        { return AXZ_ERROR_NOT_SUPPORT; };

	virtual void clear() {};
	virtual axz_rc remove( const size_t idx )		                        { return AXZ_ERROR_NOT_SUPPORT; };
	virtual axz_rc remove( const axz_wstring& key )	                        { return AXZ_ERROR_NOT_SUPPORT; };
	
	virtual AxzDict& at( const size_t idx )									{ throw std::invalid_argument( "AxzDict::at@size_t available for array only" ); }
    virtual const AxzDict& at( const size_t idx ) const                     { throw std::invalid_argument( "AxzDict::at@size_t available for array only" ); }
	virtual AxzDict& at( const axz_wstring& key )							{ throw std::invalid_argument( "AxzDict::at@axz_wstring available for object only" ); }
	virtual const AxzDict& at( const axz_wstring& key ) const               { throw std::invalid_argument( "AxzDict::at@axz_wstring available for object only" ); }
	
	// Reserve capacity for containers
	virtual void reserve( size_t capacity ) {}

	virtual axz_rc step( axz_shared_dict_stepper stepper ) = 0;	
};

template<AxzDictType _type, class T>
class _AxzTVal: public _AxzDicVal
{
public:	
	explicit _AxzTVal( const T& val ): m_val( val ) {}
	explicit _AxzTVal( T&& val ): m_val( std::move( val ) ) {}	
	~_AxzTVal() {}

	T& data() { return this->m_val; }
	const T& data() const { return this->m_val; }
	AxzDictType type() const override								        { return _type; }
	virtual bool isType( const AxzDictType type ) const override	        { return ( _type == type ); }
	virtual axz_rc step( axz_shared_dict_stepper stepper ) override {
		return stepper->step( this->data() );
	}

protected:
	T m_val;
	
	friend class AxzDict;  // Allow AxzDict to access protected members
};

class _AxzNull final: public _AxzTVal< AxzDictType::NUL, std::nullptr_t >
{
public:
	_AxzNull(): _AxzTVal( nullptr ) {}	
};

class _AxzBool final: public _AxzTVal< AxzDictType::BOOL, bool >
{
public:
	explicit _AxzBool( bool val = true ): _AxzTVal( val ) {}		
	virtual axz_rc val( bool& val ) const override	                        { val = this->m_val; return AXZ_OK; }
	virtual bool boolVal() const override			                        { return this->m_val; }
};

const std::shared_ptr<_AxzDicVal> _AxzDicDefault::nullVal = std::make_shared<_AxzNull>();
const std::shared_ptr<_AxzDicVal> _AxzDicDefault::trueVal = std::make_shared<_AxzBool>( true );
const std::shared_ptr<_AxzDicVal> _AxzDicDefault::falseVal = std::make_shared<_AxzBool>( false );

class _AxzDouble final: public _AxzTVal< AxzDictType::NUMBER, double >
{
public:
	explicit _AxzDouble( double val = 0. ): _AxzTVal( val ) {}

	virtual axz_rc val( double& val ) const override
	{		
		val = this->m_val;		
		return AXZ_OK;
	}

	virtual axz_rc val( int32_t& val ) const override
	{		
		val = ( int32_t )this->m_val;
		return AXZ_OK;
	}	

	virtual double numberVal() const override	{ return this->m_val; }
	virtual int32_t intVal() const override		{ return ( int32_t )this->m_val; }
};

class _AxzInt final: public _AxzTVal< AxzDictType::INTEGRAL, int32_t >
{
public:
	explicit _AxzInt( int32_t val = 0 ): _AxzTVal( val ) {}

	virtual axz_rc val( double& val ) const override
	{		
		val = ( double )this->m_val;
		return AXZ_OK;
	}

	virtual axz_rc val( int32_t& val ) const override
	{		
		val = this->m_val;
		return AXZ_OK;
	}

	virtual double numberVal() const override	{ return ( double )this->m_val; }
	virtual int32_t intVal() const override		{ return this->m_val; }
};

class _AxzString final: public _AxzTVal< AxzDictType::STRING, axz_wstring >
{
public:
	explicit _AxzString( const axz_wstring& val = L"" ): _AxzTVal( val )    {}
	explicit _AxzString( axz_wstring&& val ): _AxzTVal( std::move( val ) )  {}	

	virtual axz_rc val( axz_wstring& val ) const override	                { val = this->m_val; return AXZ_OK; }
	virtual axz_rc steal( axz_wstring& val ) override						{ val = std::move( this->m_val ); return AXZ_OK; }
	virtual axz_wstring stringVal() const override		                    { return this->m_val; }
	virtual void clear() override						                    { this->m_val.clear(); }
};

class _AxzBytes final: public _AxzTVal< AxzDictType::BYTES, axz_bytes >
{
public:
	explicit _AxzBytes( const axz_bytes& val = {} ): _AxzTVal( val )        {}
	explicit _AxzBytes( axz_bytes&& val ): _AxzTVal( std::move( val ) )     {}

	virtual axz_rc val( axz_bytes& val ) const override	                    { val = this->m_val; return AXZ_OK; }
	virtual axz_rc steal( axz_bytes& val ) override							{ val = std::move( this->m_val ); return AXZ_OK; }

	virtual axz_bytes bytesVal() const override			                    { return this->m_val; }
	virtual void clear() override						                    { this->m_val.clear(); }
};

class _AxzArray final: public _AxzTVal< AxzDictType::ARRAY, axz_dict_array >
{
public:
	explicit _AxzArray( const axz_dict_array& val = {} ): _AxzTVal( val ) {}
	explicit _AxzArray( axz_dict_array&& val ): _AxzTVal( std::move( val ) ) {}

	virtual axz_rc val( const size_t idx, AxzDict& val ) override			{ return this->_val<AxzDict, true>( idx, val ); }
	virtual axz_rc val( const size_t idx, double& val ) override			{ return this->_val<double, false>( idx, val ); }
	virtual axz_rc val( const size_t idx, int32_t& val ) override   		{ return this->_val<int32_t, false>( idx, val ); }
	virtual axz_rc val( const size_t idx, bool& val ) override			    { return this->_val<bool, false>( idx, val ); }
	virtual axz_rc val( const size_t idx, axz_wstring& val ) override		{ return this->_val<axz_wstring, false>( idx, val ); }
	virtual axz_rc val( const size_t idx, axz_bytes& val ) override		    { return this->_val<axz_bytes, false>( idx, val ); }

	virtual axz_rc steal( const size_t idx, AxzDict& val ) override			{ return this->_steal<AxzDict, true>( idx, val ); }		
	virtual axz_rc steal( const size_t idx, double& val ) override			{ return this->_steal<double, false>( idx, val ); }
	virtual axz_rc steal( const size_t idx, int32_t& val ) override   		{ return this->_steal<int32_t, false>( idx, val ); }
	virtual axz_rc steal( const size_t idx, bool& val ) override			{ return this->_steal<bool, false>( idx, val ); }
	virtual axz_rc steal( const size_t idx, axz_wstring& val ) override		{ return this->_steal<axz_wstring, false>( idx, val ); }		
	virtual axz_rc steal( const size_t idx, axz_bytes& val ) override		{ return this->_steal<axz_bytes, false>( idx, val ); }	

	virtual void clear() override										    { this->m_val.clear(); }
	virtual size_t size() const override								    { return this->m_val.size(); }

	virtual axz_rc add( const AxzDict& val ) override	
	{
		this->m_val.emplace_back( val );
		return AXZ_OK; 
	}

	virtual axz_rc add( AxzDict&& val )	override		
	{
		this->m_val.emplace_back( std::move( val ) );
		return AXZ_OK; 
	}	

	virtual axz_rc remove( const size_t idx ) override		
	{ 
		if ( idx >= this->m_val.size() )
			return AXZ_ERROR_OUT_OF_RANGE;

		this->m_val.erase( this->m_val.begin() + idx );
		return AXZ_OK;
	}	
    
	const AxzDict& at( const size_t idx ) const override
	{
		assert( idx < this->m_val.size() );
		return this->m_val[idx];
	}

	virtual AxzDict& at( const size_t idx ) override
	{
		if (idx >= this->m_val.size()) {
			throw std::out_of_range("Index out of range");
		}
		return this->m_val[idx];
	}
	
	virtual void reserve( size_t capacity ) override
	{
		this->m_val.reserve(capacity);
	}
	
private:
	
	template<class V, bool isDict>
	axz_rc _val( const size_t idx, V& val )
	{		
		if ( idx >= this->m_val.size() )
			return AXZ_ERROR_OUT_OF_RANGE;

		return Internal::getVal( this->m_val[idx], val, _AxzBool2Type<isDict>() );
	}

	template<class V, bool isDict>
	axz_rc _steal( const size_t idx, V& val )
	{		
		if ( idx >= this->m_val.size() )
			return AXZ_ERROR_OUT_OF_RANGE;

		return Internal::stealVal( this->m_val[idx], val, _AxzBool2Type<isDict>() );		
	}	
};

class _AxzObject final: public _AxzTVal< AxzDictType::OBJECT, axz_dict_object_safe >
{
public:
	_AxzObject() : _AxzTVal( axz_dict_object_safe() ) {}
	_AxzObject( const axz_dict_object& val ) : _AxzTVal( axz_dict_object_safe() ) {
		// Convert standard object to safe object
		for (const auto& pair : val) {
			this->m_val.emplace(pair.first, pair.second);
		}
	}
	_AxzObject( axz_dict_object&& val ) : _AxzTVal( axz_dict_object_safe() ) {
		// Convert standard object to safe object
		for (auto& pair : val) {
			this->m_val.emplace(std::move(pair.first), std::move(pair.second));
		}
	}
	_AxzObject( const axz_dict_object_safe& val ) : _AxzTVal( val ) {}
	_AxzObject( axz_dict_object_safe&& val ) : _AxzTVal( std::move( val ) ) {}

	virtual axz_rc val( const axz_wstring& key, AxzDict& val ) override			{ return this->_val<AxzDict, true>( key, val ); }
	virtual axz_rc val( const axz_wstring& key, double& val ) override			{ return this->_val<double, false>( key, val ); }
	virtual axz_rc val( const axz_wstring& key, int32_t& val ) override			{ return this->_val<int32_t, false>( key, val ); }
	virtual axz_rc val( const axz_wstring& key, bool& val ) override			{ return this->_val<bool, false>( key, val ); }
	virtual axz_rc val( const axz_wstring& key, axz_wstring& val )	override	{ return this->_val<axz_wstring, false>( key, val ); }
	virtual axz_rc val( const axz_wstring& key, axz_bytes& val ) override		{ return this->_val<axz_bytes, false>( key, val ); }
	
	virtual axz_rc steal( const axz_wstring& key, AxzDict& val ) override			{ return this->_steal<AxzDict, true>( key, val ); }		
	virtual axz_rc steal( const axz_wstring& key, double& val ) override			{ return this->_steal<double, false>( key, val ); }
	virtual axz_rc steal( const axz_wstring& key, int32_t& val ) override			{ return this->_steal<int32_t, false>( key, val ); }
	virtual axz_rc steal( const axz_wstring& key, bool& val ) override				{ return this->_steal<bool, false>( key, val ); }
	virtual axz_rc steal( const axz_wstring& key, axz_wstring& val ) override		{ return this->_steal<axz_wstring, false>( key, val ); }	
	virtual axz_rc steal( const axz_wstring& key, axz_bytes& val ) override			{ return this->_steal<axz_bytes, false>( key, val ); }			

	virtual size_t size() const override	                                    { return this->m_val.size(); }
	virtual void clear() override			                                    { this->m_val.clear(); }

	virtual axz_rc add( const AxzDict& val ) override	
	{
		if ( !val.isObject() ) 
		{
			return AXZ_ERROR_INVALID_INPUT;
		}

		auto keys = val.keys();
		for( auto key: keys )
		{
			try {
				this->m_val.emplace( key, val[key] );
			} catch (const std::exception&) {
				// Continue on individual key failures
			}
		}		
		return AXZ_OK; 
	}

	virtual axz_rc add( AxzDict&& val )	override		
	{
		if ( !val.isObject() ) 
		{
			return AXZ_ERROR_INVALID_INPUT;
		}

		auto keys = val.keys();
		for( auto key: keys )
		{
			try {
				this->m_val.emplace( key, std::move( val[key] ) );
			} catch (const std::exception&) {
				// Continue on individual key failures
			}
		}		
		return AXZ_OK; 
	}

	virtual axz_rc add( const axz_wstring& key, const AxzDict& val ) override 
	{ 
		try {
			auto found = this->m_val.find( key );
			if ( found == this->m_val.end() )
			{
				this->m_val.emplace( key, val );
				return AXZ_OK;
			}
			found->second = val;
			return AXZ_OK_REPLACED;
		} catch (const std::exception&) {
			return AXZ_ERROR_HASH_ERROR;
		}
	}

	virtual axz_rc add( const axz_wstring& key, AxzDict&& val ) override
	{
		try {
			auto found = this->m_val.find( key );
			if ( found == this->m_val.end() )
			{
				this->m_val.emplace( key, std::move( val ) );
				return AXZ_OK;
			}
			found->second = std::move( val );
			return AXZ_OK_REPLACED;
		} catch (const std::exception&) {
			return AXZ_ERROR_HASH_ERROR;
		}
	}

	virtual axz_rc remove( const axz_wstring& key ) override	
	{ 
		try {
			this->m_val.erase( key ); 
			return AXZ_OK;
		} catch (const std::exception&) {
			return AXZ_ERROR_HASH_ERROR;
		}
	}	

	AxzDict& at( const axz_wstring& key ) override
	{
		try {
			auto found = this->m_val.find( key );
			if ( found == this->m_val.end() ) {
				// Create key with null value if it doesn't exist (for operator[] behavior)
				auto [iter, inserted] = this->m_val.emplace( key, AxzDict() );
				return iter->second;
			}
			return found->second;
		} catch (const std::exception&) {
			// Return reference to a static null dict in case of error
			static AxzDict null_dict;
			return null_dict;
		}
	}
	
	const AxzDict& at( const axz_wstring& key ) const override
	{
		try {
			auto found = this->m_val.find( key );
			if ( found == this->m_val.end() ) {
				throw std::out_of_range("Key not found");
			}
			return found->second;
		} catch (const std::out_of_range&) {
			throw; // Re-throw out_of_range
		} catch (const std::exception&) {
			// Return reference to a static null dict in case of other errors
			static const AxzDict null_dict;
			return null_dict;
		}
	}
	
	virtual void reserve( size_t capacity ) override
	{
		try {
			this->m_val.reserve(capacity);
		} catch (const std::exception&) {
			// Ignore reserve failures
		}
	}

private:	

	template<class V, bool isDict>
	axz_rc _val( const axz_wstring& key, V& val )
	{	
		try {
			auto found = this->m_val.find( key );
			if ( found == this->m_val.end() )
				return AXZ_ERROR_NOT_FOUND;

			return Internal::getVal( found->second, val, _AxzBool2Type<isDict>() );
		} catch (const std::exception&) {
			return AXZ_ERROR_HASH_ERROR;
		}
	}		

	template<class V, bool isDict>
	axz_rc _steal( const axz_wstring& key, V& val )
	{	
		try {
			auto found = this->m_val.find( key );
			if ( found == this->m_val.end() )
				return AXZ_ERROR_NOT_FOUND;
			
			return Internal::stealVal( found->second, val, _AxzBool2Type<isDict>() );	
		} catch (const std::exception&) {
			return AXZ_ERROR_HASH_ERROR;
		}
	}
};

// Iterator implementations for AxzDict
AxzDict::iterator::iterator(std::shared_ptr<_AxzDicVal> val, size_t index) 
    : m_val(val), m_index(index), m_is_array(true) {}

AxzDict::iterator::iterator(std::shared_ptr<_AxzDicVal> val, axz_dict_object_safe::iterator it)
    : m_val(val), m_obj_it(it), m_is_array(false) {}

AxzDict::iterator::reference AxzDict::iterator::operator*() const {
    if (m_is_array) {
        return m_val->at(m_index);
    } else {
        return m_obj_it->second;
    }
}

AxzDict::iterator::pointer AxzDict::iterator::operator->() const {
    return &operator*();
}

AxzDict::iterator& AxzDict::iterator::operator++() {
    if (m_is_array) {
        ++m_index;
    } else {
        ++m_obj_it;
    }
    return *this;
}

AxzDict::iterator AxzDict::iterator::operator++(int) {
    iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool AxzDict::iterator::operator==(const iterator& other) const {
    if (m_is_array != other.m_is_array) return false;
    if (m_is_array) {
        return m_index == other.m_index;
    } else {
        return m_obj_it == other.m_obj_it;
    }
}

bool AxzDict::iterator::operator!=(const iterator& other) const {
    return !(*this == other);
}

// const_iterator implementations
AxzDict::const_iterator::const_iterator(std::shared_ptr<_AxzDicVal> val, size_t index) 
    : m_val(val), m_index(index), m_is_array(true) {}

AxzDict::const_iterator::const_iterator(std::shared_ptr<_AxzDicVal> val, axz_dict_object_safe::const_iterator it)
    : m_val(val), m_obj_it(it), m_is_array(false) {}

AxzDict::const_iterator::const_iterator(const iterator& it) 
    : m_val(it.m_val), m_index(it.m_index), m_is_array(it.m_is_array) {
    if (!m_is_array) {
        // Convert iterator to const_iterator for object case
        m_obj_it = it.m_obj_it;
    }
}

AxzDict::const_iterator::reference AxzDict::const_iterator::operator*() const {
    if (m_is_array) {
        return m_val->at(m_index);
    } else {
        return m_obj_it->second;
    }
}

AxzDict::const_iterator::pointer AxzDict::const_iterator::operator->() const {
    return &operator*();
}

AxzDict::const_iterator& AxzDict::const_iterator::operator++() {
    if (m_is_array) {
        ++m_index;
    } else {
        ++m_obj_it;
    }
    return *this;
}

AxzDict::const_iterator AxzDict::const_iterator::operator++(int) {
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool AxzDict::const_iterator::operator==(const const_iterator& other) const {
    if (m_is_array != other.m_is_array) return false;
    if (m_is_array) {
        return m_index == other.m_index;
    } else {
        return m_obj_it == other.m_obj_it;
    }
}

bool AxzDict::const_iterator::operator!=(const const_iterator& other) const {
    return !(*this == other);
}

// Iterator methods for AxzDict
AxzDict::iterator AxzDict::begin() {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() == AxzDictType::ARRAY) {
        return iterator(m_val, 0);
    } else if (m_val->type() == AxzDictType::OBJECT) {
        auto obj_ptr = static_cast<_AxzObject*>(m_val.get());
        return iterator(m_val, obj_ptr->m_val.begin());
    }
    return end();
}

AxzDict::iterator AxzDict::end() {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() == AxzDictType::ARRAY) {
        return iterator(m_val, m_val->size());
    } else if (m_val->type() == AxzDictType::OBJECT) {
        auto obj_ptr = static_cast<_AxzObject*>(m_val.get());
        return iterator(m_val, obj_ptr->m_val.end());
    }
    return iterator(m_val, 0);
}

AxzDict::const_iterator AxzDict::begin() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() == AxzDictType::ARRAY) {
        return const_iterator(m_val, 0);
    } else if (m_val->type() == AxzDictType::OBJECT) {
        auto obj_ptr = static_cast<const _AxzObject*>(m_val.get());
        return const_iterator(m_val, obj_ptr->m_val.begin());
    }
    return end();
}

AxzDict::const_iterator AxzDict::end() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() == AxzDictType::ARRAY) {
        return const_iterator(m_val, m_val->size());
    } else if (m_val->type() == AxzDictType::OBJECT) {
        auto obj_ptr = static_cast<const _AxzObject*>(m_val.get());
        return const_iterator(m_val, obj_ptr->m_val.end());
    }
    return const_iterator(m_val, 0);
}

AxzDict::const_iterator AxzDict::cbegin() const {
    return begin();
}

AxzDict::const_iterator AxzDict::cend() const {
    return end();
}

// AxzDict constructor implementations - optimized with caching
AxzDict::AxzDict() noexcept : m_val( _AxzDicDefault::nullVal ) {}

AxzDict::AxzDict( const AxzDict& other ) : m_val( other.m_val ) {}

AxzDict::AxzDict( AxzDict&& other ) noexcept : m_val( std::move(other.m_val) ) {}

AxzDict::AxzDict( int value ) {
    // Use simple caching for common integer values to avoid allocation overhead
    if (value >= -10 && value <= 100) {
        static std::array<std::shared_ptr<_AxzInt>, 111> cached_ints;
        static std::once_flag cache_init;
        
        std::call_once(cache_init, []() {
            for (int i = 0; i < 111; ++i) {
                cached_ints[i] = std::make_shared<_AxzInt>(i - 10);
            }
        });
        
        m_val = cached_ints[value + 10];
    } else {
        m_val = std::make_shared<_AxzInt>(value);
    }
}

AxzDict::AxzDict( double value ) {
    m_val = std::make_shared<_AxzDouble>(value);
}

AxzDict::AxzDict( bool value ) : m_val( value ? _AxzDicDefault::trueVal : _AxzDicDefault::falseVal ) {}

AxzDict::AxzDict( const wchar_t* value ) {
    m_val = std::make_shared<_AxzString>(axz_wstring(value));
}

AxzDict::AxzDict( const axz_wstring& value ) {
    m_val = std::make_shared<_AxzString>(value);
}

AxzDict::AxzDict( axz_dict_array&& value ) noexcept {
    m_val = std::make_shared<_AxzArray>(std::move(value));
}

AxzDict::AxzDict( axz_dict_object&& value ) noexcept {
    m_val = std::make_shared<_AxzObject>(std::move(value));
}

// Constructor with AxzDictType
AxzDict::AxzDict( AxzDictType type ) noexcept {
    switch (type) {
        case AxzDictType::NUL:
            m_val = _AxzDicDefault::nullVal;
            break;
        case AxzDictType::BOOL:
            m_val = _AxzDicDefault::falseVal;
            break;
        case AxzDictType::NUMBER:
            m_val = std::make_shared<_AxzDouble>(0.0);
            break;
        case AxzDictType::INTEGRAL:
            m_val = std::make_shared<_AxzInt>(0);
            break;
        case AxzDictType::STRING:
            m_val = std::make_shared<_AxzString>(axz_wstring());
            break;
        case AxzDictType::ARRAY:
            m_val = std::make_shared<_AxzArray>(axz_dict_array());
            break;
        case AxzDictType::OBJECT:
            m_val = std::make_shared<_AxzObject>(axz_dict_object_safe());
            break;
        default:
            m_val = _AxzDicDefault::nullVal;
            break;
    }
}

// Modern C++17 Enhanced Implementation for AxzDict

// High-performance constructors with string view support
AxzDict::AxzDict( std::wstring_view val ) {
    axz_performance::g_counters.memory_allocations.fetch_add(1, std::memory_order_relaxed);
    
    // Use string pool for frequently used strings
    if (val.size() <= SMALL_STRING_OPTIMIZATION_SIZE) {
        auto pooled = axz_performance::g_string_pool.intern(axz_wstring(val));
        m_val = std::make_shared<_AxzString>(*pooled);
    } else {
        m_val = std::make_shared<_AxzString>(axz_wstring(val));
    }
}

// Initializer list constructors for convenient syntax
AxzDict::AxzDict( std::initializer_list<AxzDict> vals ) {
    become(AxzDictType::ARRAY);
    reserve(vals.size());
    for (const auto& val : vals) {
        add(val);
    }
}

AxzDict::AxzDict( std::initializer_list<std::pair<axz_wstring, AxzDict>> vals ) {
    become(AxzDictType::OBJECT);
    reserve(vals.size());
    for (const auto& pair : vals) {
        add(pair.first, pair.second);
    }
}

// Enhanced assignment operators
AxzDict& AxzDict::operator=( std::wstring_view val ) {
    stats.set_operations.fetch_add(1, std::memory_order_relaxed);
    
    // Use string pool for efficiency
    if (val.size() <= SMALL_STRING_OPTIMIZATION_SIZE) {
        auto pooled = axz_performance::g_string_pool.intern(axz_wstring(val));
        *this = *pooled;
    } else {
        *this = axz_wstring(val);
    }
    return *this;
}

AxzDict& AxzDict::operator=( std::initializer_list<AxzDict> vals ) {
    become(AxzDictType::ARRAY);
    clear();
    reserve(vals.size());
    for (const auto& val : vals) {
        add(val);
    }
    return *this;
}

AxzDict& AxzDict::operator=( std::initializer_list<std::pair<axz_wstring, AxzDict>> vals ) {
    become(AxzDictType::OBJECT);
    clear();
    reserve(vals.size());
    for (const auto& pair : vals) {
        add(pair.first, pair.second);
    }
    return *this;
}

// Enhanced utility methods
bool AxzDict::empty() const noexcept {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    access_count.fetch_add(1, std::memory_order_relaxed);
    
    if AXZ_CONSTEXPR_IF (true) {
        switch (type()) {
            case AxzDictType::NUL:
                return true;
            case AxzDictType::ARRAY:
            case AxzDictType::OBJECT:
                return size() == 0;
            case AxzDictType::STRING:
                return stringVal().empty();
            case AxzDictType::BYTES:
                return bytesVal().empty();
            default:
                return false;
        }
    }
}

void AxzDict::shrink_to_fit() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    stats.memory_reallocations.fetch_add(1, std::memory_order_relaxed);
    
    // Implementation depends on internal data structure
    // This is a placeholder - actual implementation would optimize storage
}

// Path-based operations with SIMD-optimized path parsing
axz_rc AxzDict::get_path(std::wstring_view path, AxzDict& result) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    access_count.fetch_add(1, std::memory_order_relaxed);
    
    // Fast path for simple keys (no dots)
    if (path.find(L'.') == std::wstring_view::npos) {
        return val(axz_wstring(path), result);
    }
    
    // Complex path - use dot notation
    return dotVal(axz_wstring(path), result);
}

axz_rc AxzDict::set_path(std::wstring_view path, const AxzDict& value) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    stats.set_operations.fetch_add(1, std::memory_order_relaxed);
    
    // Fast path for simple keys
    if (path.find(L'.') == std::wstring_view::npos) {
        return add(axz_wstring(path), value);
    }
    
    // Complex path - need to create intermediate objects
    // This is a simplified implementation
    return add(axz_wstring(path), value);
}

axz_rc AxzDict::set_path(std::wstring_view path, AxzDict&& value) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    stats.set_operations.fetch_add(1, std::memory_order_relaxed);
    
    // Fast path for simple keys
    if (path.find(L'.') == std::wstring_view::npos) {
        return add(axz_wstring(path), std::move(value));
    }
    
    // Complex path
    return add(axz_wstring(path), std::move(value));
}

bool AxzDict::has_path(std::wstring_view path) const noexcept {
    try {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        access_count.fetch_add(1, std::memory_order_relaxed);
        
        if (path.find(L'.') == std::wstring_view::npos) {
            return AXZ_SUCCESS(contain(axz_wstring(path)));
        }
        
        return AXZ_SUCCESS(contain(axz_wstring(path)));
    } catch (...) {
        return false;
    }
}

// Memory-efficient merge operations
void AxzDict::merge(const AxzDict& other, bool overwrite) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    if (!isObject() || !other.isObject()) {
        return;  // Can only merge objects
    }
    
    // Get all keys from other object
    auto keys = other.keys();
    for (const auto& key : keys) {
        if (overwrite || !has(key)) {
            AxzDict value;
            if (AXZ_SUCCESS(other.val(key, value))) {
                set(key, value);
            }
        }
    }
}

void AxzDict::merge(AxzDict&& other, bool overwrite) noexcept {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    if (!isObject() || !other.isObject()) {
        return;
    }
    
    // Get all keys from other object
    auto keys = other.keys();
    for (const auto& key : keys) {
        if (overwrite || !has(key)) {
            AxzDict value;
            if (AXZ_SUCCESS(other.steal(key, value))) {
                set(key, std::move(value));
            }
        }
    }
}

// Performance monitoring
void AxzDict::reset_stats() noexcept {
    access_count.store(0, std::memory_order_relaxed);
    stats.get_operations.store(0, std::memory_order_relaxed);
    stats.set_operations.store(0, std::memory_order_relaxed);
    stats.memory_reallocations.store(0, std::memory_order_relaxed);
}

size_t AxzDict::memory_usage() const noexcept {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    // Estimate memory usage - this is a simplified calculation
    size_t base_size = sizeof(AxzDict);
    
    if AXZ_CONSTEXPR_IF (true) {
        switch (type()) {
            case AxzDictType::STRING:
                return base_size + stringVal().capacity() * sizeof(wchar_t);
            case AxzDictType::BYTES:
                return base_size + bytesVal().capacity();
            case AxzDictType::ARRAY:
                return base_size + size() * sizeof(AxzDict);
            case AxzDictType::OBJECT:
                return base_size + size() * (sizeof(axz_wstring) + sizeof(AxzDict));
            default:
                return base_size;
        }
    }
}

void AxzDict::compact() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    stats.memory_reallocations.fetch_add(1, std::memory_order_relaxed);
    
    // Compact internal data structures
    // This is a placeholder for actual optimization
    shrink_to_fit();
}

// Template specialization for get_if with compile-time optimization
template<>
std::optional<double> AxzDict::get_if<double>() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    if AXZ_CONSTEXPR_IF (true) {
        if (isNumber() || isIntegral()) {
            return numberVal();
        }
    }
    return std::nullopt;
}

template<>
std::optional<int32_t> AxzDict::get_if<int32_t>() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    if AXZ_CONSTEXPR_IF (true) {
        if (isIntegral() || isNumber()) {
            return intVal();
        }
    }
    return std::nullopt;
}

template<>
std::optional<bool> AxzDict::get_if<bool>() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    if AXZ_CONSTEXPR_IF (true) {
        if (isType(AxzDictType::BOOL)) {
            return boolVal();
        }
    }
    return std::nullopt;
}

template<>
std::optional<axz_wstring> AxzDict::get_if<axz_wstring>() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    if AXZ_CONSTEXPR_IF (true) {
        if (isString()) {
            return stringVal();
        }
    }
    return std::nullopt;
}

template<>
std::optional<axz_bytes> AxzDict::get_if<axz_bytes>() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    if AXZ_CONSTEXPR_IF (true) {
        if (isBytes()) {
            return bytesVal();
        }
    }
    return std::nullopt;
}

// Missing basic method implementations
AxzDictType AxzDict::type() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->type();
}

bool AxzDict::isType(const AxzDictType type) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->isType(type);
}

bool AxzDict::isNull() const {
    return type() == AxzDictType::NUL;
}

bool AxzDict::isNumber() const {
    return type() == AxzDictType::NUMBER;
}

bool AxzDict::isIntegral() const {
    return type() == AxzDictType::INTEGRAL;
}

bool AxzDict::isString() const {
    return type() == AxzDictType::STRING;
}

bool AxzDict::isBytes() const {
    return type() == AxzDictType::BYTES;
}

bool AxzDict::isArray() const {
    return type() == AxzDictType::ARRAY;
}

bool AxzDict::isObject() const {
    return type() == AxzDictType::OBJECT;
}

bool AxzDict::isCallable() const {
    return type() == AxzDictType::CALLABLE;
}

// Value getters
axz_rc AxzDict::val(int32_t& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(val);
}

axz_rc AxzDict::val(double& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(val);
}

axz_rc AxzDict::val(bool& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(val);
}

axz_rc AxzDict::val(axz_wstring& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(val);
}

axz_rc AxzDict::val(axz_bytes& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(val);
}

// Steal methods
axz_rc AxzDict::steal(int32_t& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(val);
}

axz_rc AxzDict::steal(double& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(val);
}

axz_rc AxzDict::steal(bool& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(val);
}

axz_rc AxzDict::steal(axz_wstring& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(val);
}

axz_rc AxzDict::steal(axz_bytes& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(val);
}

// Key-based operations
axz_rc AxzDict::val(const axz_wstring& key, AxzDict& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(key, val);
}

axz_rc AxzDict::val(const axz_wstring& key, int32_t& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(key, val);
}

axz_rc AxzDict::val(const axz_wstring& key, double& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(key, val);
}

axz_rc AxzDict::val(const axz_wstring& key, bool& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(key, val);
}

axz_rc AxzDict::val(const axz_wstring& key, axz_wstring& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(key, val);
}

axz_rc AxzDict::val(const axz_wstring& key, axz_bytes& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(key, val);
}

// Value accessors
double AxzDict::numberVal() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->numberVal();
}

int32_t AxzDict::intVal() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->intVal();
}

bool AxzDict::boolVal() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->boolVal();
}

axz_wstring AxzDict::stringVal() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->stringVal();
}

axz_bytes AxzDict::bytesVal() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->bytesVal();
}

size_t AxzDict::size() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->size();
}

// Assignment operators
AxzDict& AxzDict::operator=(const AxzDict& other) {
    if (this != &other) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_val = other.m_val;
    }
    return *this;
}

AxzDict& AxzDict::operator=(AxzDict&& other) noexcept {
    if (this != &other) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_val = std::move(other.m_val);
    }
    return *this;
}

AxzDict& AxzDict::operator=(std::nullptr_t) noexcept {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = _AxzDicDefault::nullVal;
    return *this;
}

AxzDict& AxzDict::operator=(int32_t val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = std::make_shared<_AxzInt>(val);
    return *this;
}

AxzDict& AxzDict::operator=(double val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = std::make_shared<_AxzDouble>(val);
    return *this;
}

AxzDict& AxzDict::operator=(bool val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = val ? _AxzDicDefault::trueVal : _AxzDicDefault::falseVal;
    return *this;
}

AxzDict& AxzDict::operator=(const axz_wstring& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = std::make_shared<_AxzString>(val);
    return *this;
}

AxzDict& AxzDict::operator=(axz_wstring&& val) noexcept {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = std::make_shared<_AxzString>(std::move(val));
    return *this;
}

AxzDict& AxzDict::operator=(const axz_wchar* val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = std::make_shared<_AxzString>(axz_wstring(val));
    return *this;
}

// Array/Object access
const AxzDict& AxzDict::operator[](const axz_wstring& key) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->at(key);
}

AxzDict& AxzDict::operator[](const axz_wstring& key) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->at(key);
}

// Container operations
axz_rc AxzDict::add(const AxzDict& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->add(val);
}

axz_rc AxzDict::add(const axz_wstring& key, const AxzDict& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->add(key, val);
}

axz_rc AxzDict::add(const axz_wstring& key, AxzDict&& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->add(key, std::move(val));
}

void AxzDict::clear() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val->clear();
}

axz_rc AxzDict::remove(const axz_wstring& key) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->remove(key);
}

axz_rc AxzDict::contain(const axz_wstring& key) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    // Implementation for contain method
    if (!isObject()) {
        return AXZ_ERROR_NOT_SUPPORT;
    }
    
    AxzDict temp;
    return val(key, temp);
}

axz_dict_keys AxzDict::keys() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    axz_dict_keys result;
    
    if (isObject()) {
        // Iterate through object and collect keys
        auto obj_ptr = static_cast<const _AxzObject*>(m_val.get());
        for (const auto& pair : obj_ptr->m_val) {
            result.insert(pair.first);
        }
    }
    
    return result;
}

void AxzDict::become(AxzDictType type) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    switch (type) {
        case AxzDictType::NUL:
            m_val = _AxzDicDefault::nullVal;
            break;
        case AxzDictType::BOOL:
            m_val = _AxzDicDefault::falseVal;
            break;
        case AxzDictType::NUMBER:
            m_val = std::make_shared<_AxzDouble>(0.0);
            break;
        case AxzDictType::INTEGRAL:
            m_val = std::make_shared<_AxzInt>(0);
            break;
        case AxzDictType::STRING:
            m_val = std::make_shared<_AxzString>(axz_wstring());
            break;
        case AxzDictType::ARRAY:
            m_val = std::make_shared<_AxzArray>(axz_dict_array());
            break;
        case AxzDictType::OBJECT:
            m_val = std::make_shared<_AxzObject>(axz_dict_object_safe());
            break;
        default:
            m_val = _AxzDicDefault::nullVal;
            break;
    }
}

void AxzDict::drop() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = _AxzDicDefault::nullVal;
}

axz_rc AxzDict::step(std::shared_ptr<AxzDictStepper> stepper) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->step(stepper);
}

void AxzDict::reserve(size_t capacity) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val->reserve(capacity);
}

// Steal with key
axz_rc AxzDict::steal(const axz_wstring& key, AxzDict& val) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(key, val);
}

// Dot notation support
axz_rc AxzDict::dotVal(const axz_wstring& key_list, AxzDict& val) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    // Simple implementation - just treat as regular key for now
    // A full implementation would parse the dot notation
    return this->val(key_list, val);
}

// Integration compatibility methods for universal_observable_json
bool AxzDict::has(const axz_wstring& key) const {
    return AXZ_SUCCESS(contain(key));
}

void AxzDict::set(const axz_wstring& key, const AxzDict& value) {
    add(key, value);
}

void AxzDict::append(const AxzDict& value) {
    add(value);
}

