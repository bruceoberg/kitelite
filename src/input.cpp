#include "input.h"

#include "etl/vector.h"
#include "etl/queue.h"

namespace Input
{
	struct SKeyStamps // tag: keys
	{
				SKeyStamps(USEC usec = UsecNow())
				: m_usUp(usec),
				  m_usDown(usec)
					{ ; }

		void	SetDown(bool fDown)
					{
						if (fDown) 
						{
							m_usDown.Reset();
						}
						else
						{
							m_usUp.Reset();
						}
					}
		bool	FIsDown() const
					{ return m_usUp.Usec() < m_usDown.Usec(); }

		CUpStamp	m_usUp;		// last seen up state
		CUpStamp	m_usDown;	// last seen down state
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

void Input::Push(const SEvent &event)
{
}

void Input::Update()
{
}

