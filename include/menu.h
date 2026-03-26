#pragma once

#include "common.h"

struct SMenu; // tag: menu

struct SMenuItem // tag: menui
{
	const char *		m_pChzLabel;		// static label text, or nullptr
	const char *		(*m_pfnLabel)();	// dynamic label callback, or nullptr; takes priority over m_pChzLabel
	void				(*m_pfnAction)();	// leaf action callback, or nullptr
	const SMenu *		m_pMenuSub;			// submenu, or nullptr
};

struct SMenu // tag: menu
{
	U8					m_cMenui;
	const SMenuItem *	m_aMenui;
};

class CMenuNav // tag: menunav
{
public:
					CMenuNav();

	void			Push(const SMenu * pMenu);
	void			Pop();
	void			ScrollNext();
	void			Activate();

	bool			FIsEmpty() const;
	const SMenu *	PMenuCur() const;
	U8				IItemCur() const;

private:

	struct SFrame // tag: frame
	{
		const SMenu *	m_pMenu;
		U8				m_iItem;			// cursor position within m_pMenu
	};

	static const int s_cFrameMax = 8;

	SFrame			m_aFrame[s_cFrameMax];
	U8				m_cFrame;
};
