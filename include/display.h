#pragma once

#include "common.h"

#define ENABLE_DISPLAY PLAT_FEATHER_S3_REVTFT

namespace Display
{
#if ENABLE_DISPLAY
	void Startup();
	void Update();
#else // !ENABLE_DISPLAY
	inline void Startup() { ; };
	inline void Update() { ; };
#endif // !ENABLE_DISPLAY
}