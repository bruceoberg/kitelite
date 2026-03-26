#include "input.h"
#include "screen.h"

#include "etl/vector.h"
#include "etl/queue.h"

namespace Input
{
	struct SKeyStamps // tag: keys
	{
				SKeyStamps(USEC usec = UsecNow())
				: m_usUp(usec),
				  m_usDown(usec),
				  m_usecFlipMade(usec),
				  m_usecFlipUsed(usec)
					{ ; }

		void	SetDown(bool fDown)
					{
						USEC usec = UsecNow();
						bool fWasDown = FIsDown();

						if (fDown)
						{
							m_usDown.Reset(usec);
						}
						else
						{
							m_usUp.Reset(usec);
						}

						// BB bruceo: my kingdom for logical xor

						if (fWasDown != FIsDown())
						{
							m_usecFlipMade = usec;
						}
					}
		bool	FIsDown() const
					{ return m_usUp.Usec() < m_usDown.Usec(); }
		bool	FHasFlip() const
					{ return m_usecFlipMade > m_usecFlipUsed; }
		void	UseFlip(USEC usec = UsecNow())
					{ m_usecFlipUsed = usec; }

		CUpStamp	m_usUp;		// last seen up state
		CUpStamp	m_usDown;	// last seen down state
		USEC		m_usecFlipMade;	// time we detected a flipped up/down state
		USEC		m_usecFlipUsed;	// time we consumed said flip.
	};

	etl::vector<SKeyStamps, Input::KEY_Max> g_aryKeys;
	etl::queue<Input::SEvent, 32> g_qEvent;
}

using namespace Input;

void Input::Startup()
{
	g_aryKeys.fill(SKeyStamps());
}

void Input::Register(IEventSync * pEventsync)
{
}

void Input::SetKeyDown(KEY key, bool fDown)
{
	g_aryKeys[key].SetDown(fDown);
}

bool Input::FIsKeyDown(KEY key)
{
	return g_aryKeys[key].FIsDown();
}

USEC Input::UsecKeyDown(KEY key)
{
	return g_aryKeys[key].m_usDown.Usec();
}

void Input::Push(const SEvent &event)
{
	if (!g_qEvent.full())
		g_qEvent.push(event);
}

void Input::Update()
{
	// edge detection: compare current key state against previous frame

	USEC usec = UsecNow();

	for (int iKey = 0; iKey < Input::KEY_Max; ++iKey)
	{
		KEY key = static_cast<KEY>(iKey);
		
		if (!g_aryKeys[key].FHasFlip())
			continue;

		g_aryKeys[key].UseFlip();

		if (g_aryKeys[key].FIsDown())
		{
			Push(SEvent(EVENTK_KeyDown, usec, key));
			Push(SEvent(EVENTK_KeyPressed, usec, key));
		}
		else
		{
			Push(SEvent(EVENTK_KeyUp, usec, key));
		}
	}

	// drain the event queue

	while (!g_qEvent.empty())
	{
		Input::SEvent event;
		g_qEvent.pop_into(event);
		Screen::DispatchInput(event);
	}
}

