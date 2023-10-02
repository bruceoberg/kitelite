#pragma once

#include "common.h"

namespace Trace
{
	void Startup();

	template<class... Args>
	inline
	void Printf(const char * pChzFormat, Args&&... args)
				{ Serial.printf(pChzFormat, std::forward<Args>(args)...); }

	template<class... Args>
	inline
	void Printf(bool fTrace, const char * pChzFormat, Args&&... args)
				{ 
					if (!fTrace)
						return;
				    
					Serial.printf(pChzFormat, std::forward<Args>(args)...);
				}
}

#define TRACE Trace::Printf