#ifndef __axz_dict_stepper__
#define __axz_dict_stepper__

#include "axz_types.h"
#include "axz_export.h"
#include "axz_error_codes.h"
#include <memory>

#ifdef _MSC_VER
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

class AxzDictStepper;
using axz_shared_dict_stepper    = std::shared_ptr<AxzDictStepper>;
using axz_weak_dict_stepper      = std::weak_ptr<AxzDictStepper>;

//class AxzDictStepper;
//template class AXZDICT_DECLCLASS std::weak_ptr<AxzDictStepper>;

class AXZDICT_DECLCLASS AxzDictStepper: public std::enable_shared_from_this<AxzDictStepper>
{
public:
	AxzDictStepper(): std::enable_shared_from_this<AxzDictStepper>() {};
	virtual ~AxzDictStepper() = default;

	virtual axz_rc step( std::nullptr_t)				{ return AXZ_OK; }
	virtual axz_rc step( const bool )					{ return AXZ_OK; }
	virtual axz_rc step( const int32_t )				{ return AXZ_OK; }
	virtual axz_rc step( const double )					{ return AXZ_OK; }
	virtual axz_rc step( const axz_wstring& )			{ return AXZ_OK; }
	virtual axz_rc step( const axz_bytes& )				{ return AXZ_OK; }
	virtual axz_rc step( const axz_dict_array& )	    { return AXZ_OK; }
	virtual axz_rc step( const axz_dict_object& )	    { return AXZ_OK; }
	
	// Support for safe object with ultra-fast hash/equal functions
	virtual axz_rc step( const std::unordered_map<axz_wstring, class AxzDict, struct axz_hash_internal::UltraFastWStringHash, struct axz_hash_internal::UltraFastWStringEqual>& safe_obj ) { 
		// Convert safe object to regular object for compatibility
		axz_dict_object regular_obj;
		for (const auto& pair : safe_obj) {
			regular_obj.emplace(pair.first, pair.second);
		}
		return step(regular_obj);
	}
};

#ifdef _MSC_VER
#	pragma warning( pop )
#endif

#endif
