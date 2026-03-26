#pragma once

#include "common.h"
#include "input.h"

// IScreen — base interface for screen stack entries

class IScreen // tag: screen
{
public:
	virtual			~IScreen() = default;

	virtual void	Update() = 0;
	virtual void	OnInput(Input::SEvent event) = 0;
	virtual void	OnPush()	{ ; }
	virtual void	OnPop()		{ ; }
	virtual void	OnCover()	{ ; }
	virtual void	OnUncover()	{ ; }
};

namespace Screen
{
	void Startup();
	void Update();

	void Push(IScreen * pScreen);
	void Pop();

	bool FIsActive(IScreen * pScreen);

	void DispatchInput(Input::SEvent event);
} // namespace Screen
