#ifndef __axz_native__
#define __axz_native__

#include <stdint.h>

//-- we must seperate axz_native from axz_types in order not to border objective-c because of c++ features
//-- we must use "typedef" instead of "using" because objective-c doesn't understand the new key

typedef uint8_t axz_byte;
typedef char axz_char;
typedef wchar_t axz_wchar;
typedef int32_t axz_rc;
typedef int32_t axz_int;

#endif