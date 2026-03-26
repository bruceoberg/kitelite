#include "menu.h"

CMenuNav::CMenuNav()
: m_cFrame(0)
{
	ClearArray(m_aFrame);
}

void CMenuNav::Push(const SMenu * pMenu)
{
	if (!pMenu || m_cFrame >= s_cFrameMax)
		return;

	SFrame & frame = m_aFrame[m_cFrame];
	frame.m_pMenu = pMenu;
	frame.m_iItem = 0;
	++m_cFrame;
}

void CMenuNav::Pop()
{
	if (m_cFrame > 0)
		--m_cFrame;
}

void CMenuNav::ScrollNext()
{
	if (m_cFrame == 0)
		return;

	SFrame & frame = m_aFrame[m_cFrame - 1];
	frame.m_iItem = (frame.m_iItem + 1) % frame.m_pMenu->m_cMenui;
}

void CMenuNav::Activate()
{
	if (m_cFrame == 0)
		return;

	const SFrame & frame = m_aFrame[m_cFrame - 1];
	const SMenuItem & menui = frame.m_pMenu->m_aMenui[frame.m_iItem];

	if (menui.m_pMenuSub)
	{
		Push(menui.m_pMenuSub);
	}
	else if (menui.m_pfnAction)
	{
		menui.m_pfnAction();
	}
}

bool CMenuNav::FIsEmpty() const
{
	return m_cFrame == 0;
}

const SMenu * CMenuNav::PMenuCur() const
{
	if (m_cFrame == 0)
		return nullptr;

	return m_aFrame[m_cFrame - 1].m_pMenu;
}

U8 CMenuNav::IItemCur() const
{
	if (m_cFrame == 0)
		return 0;

	return m_aFrame[m_cFrame - 1].m_iItem;
}
