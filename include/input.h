#pragma once

#include "common.h"
#include "clock.h"

namespace Input
{

	enum EVENTK // event kind
	{
		EVENTK_KeyDown,
		EVENTK_KeyUp,
		EVENTK_KeyPressed,

		EVENTK_Max,
		EVENTK_Nil = -1
	};

	enum KEY
	{
		KEY_OnBoard0,
		KEY_OnBoard1,
		KEY_OnBoard2,

		KEY_Max,
		KEY_Nil = -1
	};

	struct SEvent // tag: event
	{
				SEvent(
					EVENTK eventk = EVENTK_Nil,
					USEC usec = USEC_Nil,
					KEY key = KEY_Nil,
					Wch wch = 0)
				: m_eventk(eventk),
				m_usec(usec),
				m_key(key),
				m_wch(wch)
					{ ; }

		EVENTK	m_eventk;
		USEC	m_usec;
		KEY		m_key;
		Wch		m_wch;
	};

	class IEventSync // tag: eventsync
	{
	public:
		virtual bool FHandleEvent(const SEvent & event) = 0;
	};

	void Startup();
	void Register(IEventSync * pEventsync);

	void SetKeyDown(KEY key, bool fDown);
	bool FIsKeyDown(KEY key);
	USEC UsecKeyDown(KEY key);

	void Push(const SEvent & event);

	void Update();
} // namespace Input