#include "clock.h"

TICK Clock::g_tickFrame = 0;
TICK Clock::g_tickFramePrev = 0;
float Clock::g_tFrame = 0.0f;
float Clock::g_tFramePrev = 0.0f;
float Clock::g_dTFrame = 0.0f;

void Clock::Update()
{
	g_tickFramePrev = g_tickFrame;
	g_tFramePrev = g_tFrame;

	g_tickFrame = TickNow();
	g_tFrame = TFromTick(g_tickFrame);
	
	g_dTFrame = g_tFrame - g_tFramePrev;
}
