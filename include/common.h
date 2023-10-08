#pragma once

#include <Arduino.h>

// common types & macros

using U8 =	uint8_t;
using U16 =	uint16_t;
using U32 =	uint32_t;
using U64 =	uint64_t;

using S8 =	int8_t;
using S16 =	int16_t;
using S32 =	int32_t;
using S64 =	int64_t;

using Ch = unsigned char;
using Wch = wchar_t;

template<U32 N, class T>
constexpr U32 DIM(T(&)[N]) { return N; }



// no-op function to force breakpointable points in the code

void Force();