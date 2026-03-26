#include "screen.h"
#include <assert.h>

// SScreenFrame — single entry in the screen stack

struct SScreenFrame // tag: screenf
{
	IScreen *	m_pScreen;		// owning screen (not owned, caller manages lifetime)
};

// CScreenMgr — fixed-depth screen stack

class CScreenMgr // tag: screenmgr
{
public:
	static constexpr int s_cScreenfMax = 8;

	void Startup()
	{
		m_cScreenf = 0;
		ClearArray(m_aScreenf);
	}

	void Update()
	{
		IScreen * pScreen = PScreenTop();
		if (!pScreen)
			return;

		pScreen->Update();
	}

	void Push(IScreen * pScreen)
	{
		assert(pScreen);
		assert(m_cScreenf < s_cScreenfMax);

		// cover the current top, if any

		IScreen * pScreenPrev = PScreenTop();
		if (pScreenPrev)
			pScreenPrev->OnCover();

		m_aScreenf[m_cScreenf].m_pScreen = pScreen;
		m_cScreenf++;

		pScreen->OnPush();
	}

	void Pop()
	{
		assert(m_cScreenf > 0);

		IScreen * pScreen = PScreenTop();
		m_cScreenf--;
		m_aScreenf[m_cScreenf].m_pScreen = nullptr;

		if (pScreen)
			pScreen->OnPop();

		// uncover the new top, if any

		IScreen * pScreenNew = PScreenTop();
		if (pScreenNew)
			pScreenNew->OnUncover();
	}

	void DispatchInput(Input::SEvent event)
	{
		IScreen * pScreen = PScreenTop();
		if (!pScreen)
			return;

		pScreen->OnInput(event);
	}

private:

	IScreen * PScreenTop()
	{
		if (m_cScreenf <= 0)
			return nullptr;

		return m_aScreenf[m_cScreenf - 1].m_pScreen;
	}

	int				m_cScreenf;						// current stack depth
	SScreenFrame	m_aScreenf[s_cScreenfMax];		// fixed-depth stack
};

static CScreenMgr g_screenmgr;

// Screen:: namespace delegates

void Screen::Startup()
{
	g_screenmgr.Startup();
}

void Screen::Update()
{
	g_screenmgr.Update();
}

void Screen::Push(IScreen * pScreen)
{
	g_screenmgr.Push(pScreen);
}

void Screen::Pop()
{
	g_screenmgr.Pop();
}

void Screen::DispatchInput(Input::SEvent event)
{
	g_screenmgr.DispatchInput(event);
}
