#pragma once

#include "display.h"
#include "screen.h"

#if ENABLE_DISPLAY

class CScreenCalib : public IScreen // tag: screencalib
{
public:
	void	Update() override;
	void	OnInput(Input::SEvent event) override;
	void	OnPush() override;
	void	OnUncover() override;

private:
	bool	m_fDirty;
};

namespace Screen
{
	IScreen * Calib();
}

#endif // ENABLE_DISPLAY
