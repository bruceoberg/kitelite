#pragma once

#define ENABLE_BLUETOOTH 0

namespace BlueTooth
{
#if ENABLE_BLUETOOTH
	void Startup();
	void Update();
#else // !ENABLE_BLUETOOTH
	inline void Startup() { ; };
	inline void Update() { ; };
#endif // !ENABLE_BLUETOOTH
}