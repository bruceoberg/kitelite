#pragma once

#define ENABLE_MOTION 1

namespace Motion
{
#if ENABLE_MOTION
	void Startup();
	void Update();
	bool FIsSpecial();
#else // !ENABLE_MOTION
	inline void Startup() { ; };
	inline void Update() { ; };
	bool FIsSpecial() { return false; }
#endif // !ENABLE_MOTION
}