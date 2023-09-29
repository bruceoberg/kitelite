#pragma once

#include <Arduino.h>

// common types & macros

typedef uint8_t		U8;
typedef uint16_t	U16;
typedef uint32_t	U32;
typedef uint64_t	U64;

typedef int8_t		S8;
typedef int16_t		S16;
typedef int32_t		S32;
typedef int64_t		S64;

template<U32 N, class T>
constexpr U32 DIM(T(&)[N]) { return N; }

// no-op function to force breakpointable points in the code

void Force();