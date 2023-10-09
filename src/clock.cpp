#include "clock.h"

USEC Clock::g_usecFrame = 0;
USEC Clock::g_usecFramePrev = 0;
float Clock::g_tFrame = 0.0f;
float Clock::g_tFramePrev = 0.0f;
float Clock::g_dTFrame = 0.0f;

void Clock::Update()
{
	g_usecFramePrev = g_usecFrame;
	g_tFramePrev = g_tFrame;

	g_usecFrame = UsecNow();
	g_tFrame = TFromUsec(g_usecFrame);
	
	g_dTFrame = g_tFrame - g_tFramePrev;
}
