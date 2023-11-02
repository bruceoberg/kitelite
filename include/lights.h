#pragma once

#define ENABLE_LIGHTS 0

namespace Lights
{
#if ENABLE_LIGHTS
	void Startup();
	void Update();
#else // !ENABLE_LIGHTS
	inline void Startup() { ; };
	inline void Update() { ; };
#endif // !ENABLE_LIGHTS
}