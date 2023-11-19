#pragma once

#include <Arduino.h>

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32_V2)
	#define PLAT_FEATHER_V2 1
	#define PLAT_FEATHER_S3 0
	#define PLAT_FEATHER_S3_REVTFT 0
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3)
	#define PLAT_FEATHER_V2 0
	#define PLAT_FEATHER_S3 1
	#define PLAT_FEATHER_S3_REVTFT 0
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3_REVTFT)
	#define PLAT_FEATHER_V2 0
	#define PLAT_FEATHER_S3 0
	#define PLAT_FEATHER_S3_REVTFT 1
#else
#error Unknown board
#endif

#if defined(FORCE_ENABLE_TFT)
	#undef ENABLE_TFT
	#define ENABLE_TFT FORCE_ENABLE_TFT
#endif // FORCE_ENABLE_TFT

#if !defined(ENABLE_TFT)
	#define ENABLE_TFT PLAT_FEATHER_S3_REVTFT
#endif // !defined(ENABLE_TFT)



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



// Clearing

inline void ClearAb(size_t cB, void * pB)
{
	memset(pB, 0, cB);
}

template<U32 N, class T>
inline void ClearArray(T (& aT)[N] )
{
	ClearAb(N * sizeof(T), aT);
}


// no-op function to force breakpointable points in the code

void Force();