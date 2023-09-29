#pragma once

#include "common.h"

#include <esp_cpu.h>

typedef S32 TICK;

inline TICK TickNow()
{
	return esp_timer_get_time();
}

inline float TFromTick(TICK tick)
{
	return float(tick) / 1e6f;
}

namespace Clock
{
	void Update();

	extern TICK g_tickFrame;
	extern TICK g_tickFramePrev;
	extern float g_tFrame;
	extern float g_tFramePrev;
	extern float g_dTFrame;
}