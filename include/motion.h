#pragma once

#define ENABLE_MOTION 1

namespace Motion
{
#if ENABLE_MOTION
	void Startup();
	void Update();
#else // !ENABLE_MOTION
	inline void Startup() { ; };
	inline void Update() { ; };
#endif // !ENABLE_MOTION
}