#pragma once

#include "display.h"
#include "screen.h"

#if ENABLE_DISPLAY

class CScreenAbout : public IScreen // tag: screenabout
{
public:
	void	Update() override;
	void	OnInput(Input::SEvent event) override;
};

namespace Screen
{
	IScreen * About();
}

#endif // ENABLE_DISPLAY
