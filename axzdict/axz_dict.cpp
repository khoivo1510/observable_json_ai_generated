#include "axz_dict.h"
#include "axz_dict_stepper.h"
#include "axz_error_codes.h"
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <shared_mutex>

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

class _AxzObject final: public _AxzTVal< AxzDictType::OBJECT, axz_dict_object >
{
public:
	_AxzObject( const axz_dict_object& val = {} ): _AxzTVal( val )        {}
	_AxzObject( axz_dict_object&& val ): _AxzTVal( std::move( val ) )     {}

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
			this->m_val.emplace( key, val[key] );
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
			this->m_val.emplace( key, std::move( val[key] ) );
		}		
		return AXZ_OK; 
	}

	virtual axz_rc add( const axz_wstring& key, const AxzDict& val ) override 
	{ 
		auto found = this->m_val.find( key );
		if ( found == this->m_val.end() )
		{
			this->m_val.emplace( key, val );
			return AXZ_OK;
		}
		found->second = val;
		return AXZ_OK_REPLACED;
	}

	virtual axz_rc add( const axz_wstring& key, AxzDict&& val ) override
	{
		auto found = this->m_val.find( key );
		if ( found == this->m_val.end() )
		{
			this->m_val.emplace( key, std::move( val ) );
			return AXZ_OK;
		}
		found->second = std::move( val );
		return AXZ_OK_REPLACED;
	}

	virtual axz_rc remove( const axz_wstring& key ) override	
	{ 
		this->m_val.erase( key ); 
		return AXZ_OK; 
	}	

	AxzDict& at( const axz_wstring& key ) override
	{
		auto found = this->m_val.find( key );
		if ( found == this->m_val.end() ) {
			// Create key with null value if it doesn't exist (for operator[] behavior)
			auto [iter, inserted] = this->m_val.emplace( key, AxzDict() );
			return iter->second;
		}
		return found->second;
	}
	
	const AxzDict& at( const axz_wstring& key ) const override
	{
		auto found = this->m_val.find( key );
		if ( found == this->m_val.end() ) {
			throw std::out_of_range("Key not found");
		}
		return found->second;
	}
	
	virtual void reserve( size_t capacity ) override
	{
		this->m_val.reserve(capacity);
	}

private:	

	template<class V, bool isDict>
	axz_rc _val( const axz_wstring& key, V& val )
	{	
		auto found = this->m_val.find( key );
		if ( found == this->m_val.end() )
			return AXZ_ERROR_NOT_FOUND;

		return Internal::getVal( found->second, val, _AxzBool2Type<isDict>() );		
	}		

	template<class V, bool isDict>
	axz_rc _steal( const axz_wstring& key, V& val )
	{	
		auto found = this->m_val.find( key );
		if ( found == this->m_val.end() )
			return AXZ_ERROR_NOT_FOUND;
		
		return Internal::stealVal( found->second, val, _AxzBool2Type<isDict>() );	
	}

	friend class _AxzDot;
};

// Iterator implementations for AxzDict
AxzDict::iterator::iterator(std::shared_ptr<_AxzDicVal> val, size_t index) 
    : m_val(val), m_index(index), m_is_array(true) {}

AxzDict::iterator::iterator(std::shared_ptr<_AxzDicVal> val, axz_dict_object::iterator it)
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

AxzDict::const_iterator::const_iterator(std::shared_ptr<_AxzDicVal> val, axz_dict_object::const_iterator it)
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

// AxzDict constructor implementations
AxzDict::AxzDict() noexcept : m_val( _AxzDicDefault::nullVal ) {}

AxzDict::AxzDict( const AxzDict& other ) : m_val( other.m_val ) {}

AxzDict::AxzDict( AxzDict&& other ) : m_val( std::move(other.m_val) ) {}

AxzDict::AxzDict( int value ) : m_val( std::make_shared<_AxzInt>( value ) ) {}

AxzDict::AxzDict( double value ) : m_val( std::make_shared<_AxzDouble>( value ) ) {}

AxzDict::AxzDict( bool value ) : m_val( value ? _AxzDicDefault::trueVal : _AxzDicDefault::falseVal ) {}

AxzDict::AxzDict( const wchar_t* value ) : m_val( std::make_shared<_AxzString>( axz_wstring(value) ) ) {}

AxzDict::AxzDict( const axz_wstring& value ) : m_val( std::make_shared<_AxzString>( value ) ) {}

AxzDict::AxzDict( axz_dict_array&& value ) : m_val( std::make_shared<_AxzArray>( std::move(value) ) ) {}

AxzDict::AxzDict( axz_dict_object&& value ) : m_val( std::make_shared<_AxzObject>( std::move(value) ) ) {}

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
            m_val = std::make_shared<_AxzObject>(axz_dict_object());
            break;
        default:
            m_val = _AxzDicDefault::nullVal;
            break;
    }
}

// Assignment operators
AxzDict& AxzDict::operator=( const AxzDict& other ) {
    if (this != &other) {
        m_val = other.m_val;
    }
    return *this;
}

AxzDict& AxzDict::operator=( AxzDict&& other ) {
    if (this != &other) {
        m_val = std::move(other.m_val);
    }
    return *this;
}

AxzDict& AxzDict::operator=( int value ) {
    m_val = std::make_shared<_AxzInt>( value );
    return *this;
}

AxzDict& AxzDict::operator=( double value ) {
    m_val = std::make_shared<_AxzDouble>( value );
    return *this;
}

AxzDict& AxzDict::operator=( bool value ) {
    m_val = value ? _AxzDicDefault::trueVal : _AxzDicDefault::falseVal;
    return *this;
}

AxzDict& AxzDict::operator=( const wchar_t* value ) {
    m_val = std::make_shared<_AxzString>( axz_wstring(value) );
    return *this;
}

// Missing assignment operator for wstring move
AxzDict& AxzDict::operator=( axz_wstring&& value ) {
    m_val = std::make_shared<_AxzString>( std::move(value) );
    return *this;
}

// Enhanced operator[] implementations
AxzDict& AxzDict::operator[]( const axz_wstring& key ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    if (m_val->type() == AxzDictType::NUL) {
        // Transform null to object
        m_val = std::make_shared<_AxzObject>( axz_dict_object() );
    }
    
    if (m_val->type() != AxzDictType::OBJECT) {
        throw std::out_of_range("AxzDict::operator[](key) called on non-object type");
    }
    
    return m_val->at(key);
}

AxzDict& AxzDict::operator[]( size_t index ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    if (m_val->type() == AxzDictType::NUL) {
        // Transform null to array
        m_val = std::make_shared<_AxzArray>( axz_dict_array() );
    }
    
    if (m_val->type() != AxzDictType::ARRAY) {
        throw std::out_of_range("AxzDict::operator[](index) called on non-array type");
    }
    
    // Expand array if necessary
    auto arr_ptr = static_cast<_AxzArray*>(m_val.get());
    if (index >= arr_ptr->m_val.size()) {
        arr_ptr->m_val.resize(index + 1);
    }
    
    return m_val->at(index);
}

// Utility methods
bool AxzDict::empty() const {
    return m_val->size() == 0;
}

void AxzDict::reserve( size_t capacity ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val->reserve(capacity);
}

// Remaining basic methods that were in the original interface
void AxzDict::become( AxzDictType type ) {
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
            m_val = std::make_shared<_AxzObject>(axz_dict_object());
            break;
        default:
            // Just ignore unsupported types
            break;
    }
}

void AxzDict::clear() {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_val = _AxzDicDefault::nullVal;
}

void AxzDict::drop() {
    clear();
}

axz_rc AxzDict::add( const AxzDict& val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->add(val);
}

axz_rc AxzDict::add( const axz_wstring& key, AxzDict&& val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->add(key, std::move(val));
}

axz_rc AxzDict::add( const axz_wstring& key, const AxzDict& val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->add(key, val);
}

// Missing key-based val method
axz_rc AxzDict::val( const axz_wstring& key, AxzDict& val ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(key, val);
}

// Missing contain method
axz_rc AxzDict::contain( const axz_wstring& key ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() != AxzDictType::OBJECT) {
        return AXZ_ERROR_NOT_SUPPORT;
    }
    
    auto obj_ptr = static_cast<const _AxzObject*>(m_val.get());
    auto it = obj_ptr->m_val.find(key);
    return (it != obj_ptr->m_val.end()) ? AXZ_OK : AXZ_ERROR_NOT_FOUND;
}

// Missing remove method
axz_rc AxzDict::remove( const axz_wstring& key ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() != AxzDictType::OBJECT) {
        return AXZ_ERROR_NOT_SUPPORT;
    }
    
    auto obj_ptr = static_cast<_AxzObject*>(m_val.get());
    auto it = obj_ptr->m_val.find(key);
    if (it != obj_ptr->m_val.end()) {
        obj_ptr->m_val.erase(it);
        return AXZ_OK;
    }
    return AXZ_ERROR_NOT_FOUND;
}

axz_rc AxzDict::step( std::shared_ptr<AxzDictStepper> stepper ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->step(stepper);
}

axz_rc AxzDict::val( int& out_val ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(out_val);
}

axz_rc AxzDict::val( double& out_val ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(out_val);
}

axz_rc AxzDict::val( bool& out_val ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(out_val);
}

axz_rc AxzDict::val( axz_wstring& out_val ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(out_val);
}

axz_rc AxzDict::val( axz_bytes& out_val ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->val(out_val);
}

axz_rc AxzDict::steal( int& out_val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(out_val);
}

axz_rc AxzDict::steal( double& out_val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(out_val);
}

axz_rc AxzDict::steal( bool& out_val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(out_val);
}

axz_rc AxzDict::steal( axz_wstring& out_val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(out_val);
}

axz_rc AxzDict::steal( axz_bytes& out_val ) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    return m_val->steal(out_val);
}

// Basic value accessor methods
int32_t AxzDict::intVal() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->intVal();
}

double AxzDict::numberVal() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->numberVal();
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

// Type checking methods
bool AxzDict::isNull() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->isType(AxzDictType::NUL);
}

bool AxzDict::isNumber() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->isType(AxzDictType::NUMBER);
}

bool AxzDict::isIntegral() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->isType(AxzDictType::INTEGRAL);
}

bool AxzDict::isString() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->isType(AxzDictType::STRING);
}

bool AxzDict::isArray() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->isType(AxzDictType::ARRAY);
}

bool AxzDict::isObject() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->isType(AxzDictType::OBJECT);
}

// Additional methods required by the library
size_t AxzDict::size() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->size();
}

AxzDictType AxzDict::type() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_val->type();
}

axz_dict_keys AxzDict::keys() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() != AxzDictType::OBJECT) {
        return axz_dict_keys{};
    }
    
    auto obj_ptr = static_cast<const _AxzObject*>(m_val.get());
    axz_dict_keys result;
    for (const auto& pair : obj_ptr->m_val) {
        result.insert(pair.first);
    }
    return result;
}

// Const version of operator[] 
const AxzDict& AxzDict::operator[]( const axz_wstring& key ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() != AxzDictType::OBJECT) {
        throw std::out_of_range("AxzDict::operator[](key) const called on non-object type");
    }
    return m_val->at(key);
}

const AxzDict& AxzDict::operator[]( size_t index ) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    if (m_val->type() != AxzDictType::ARRAY) {
        throw std::out_of_range("AxzDict::operator[](index) const called on non-array type");
    }
    return m_val->at(index);
}

// Integration compatibility methods
bool AxzDict::has(const axz_wstring& key) const {
    return AXZ_SUCCESS(contain(key));
}

void AxzDict::set(const axz_wstring& key, const AxzDict& value) {
    add(key, value);
}

void AxzDict::append(const AxzDict& value) {
    add(value);
}

