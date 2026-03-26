#pragma once

#include "display.h"
#include "screen.h"

#if ENABLE_DISPLAY

class CScreenHome : public IScreen // tag: screenhome
{
public:
	void	Update() override;
	void	OnInput(Input::SEvent event) override;
};

#endif // ENABLE_DISPLAY
