#pragma once

#include "common.h"

#include <esp_cpu.h>

typedef S64 USEC;

static const USEC USEC_Epoch = 0;
static const USEC USEC_Max = INT64_MAX;
static const USEC USEC_Nil = -1;

inline USEC UsecNow()
{
	return esp_timer_get_time();
}

inline constexpr float TFromUsec(USEC usec)
{
	return float(usec) / 1e6f;
}

inline constexpr USEC UsecFromT(float t)
{
	return USEC(t * 1e6f);
}

inline constexpr float DTFromDUsec(USEC dUsec)
{
	return TFromUsec(dUsec);
}

inline constexpr USEC DUsecFromDT(float dT)
{
	return UsecFromT(dT);
}

inline float TNow()
{
	return TFromUsec(UsecNow());
}

namespace Clock
{
	void Update();

	extern USEC g_usecFrame;
	extern USEC g_usecFramePrev;
	extern float g_tFrame;
	extern float g_tFramePrev;
	extern float g_dTFrame;

	inline U32 CHeartBeat(float dT)
	{
		return U32(g_tFrame / dT);
	}

	inline U32 CHeartBeatPrev(float dT)
	{
		return U32(g_tFramePrev / dT);
	}

	inline bool FHeartBeat(float dT)
	{
		return (CHeartBeat(dT) != CHeartBeatPrev(dT));
	}
}

class CUpStamp // tag: us
{
public:
				CUpStamp(USEC usec = UsecNow())
				: m_usec(usec)
					{ ; }

	void	Reset(USEC usec = UsecNow())
				{ m_usec = usec; }

	USEC	Usec() const
				{ return m_usec; }

	USEC	DUsec() const
				{ return UsecNow() - m_usec; }
	float	DT() const
				{ return DTFromDUsec(DUsec()); }

protected:
	USEC	m_usec;
};