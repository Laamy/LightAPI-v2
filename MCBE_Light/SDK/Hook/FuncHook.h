#pragma once

class FuncHook {
public:
	virtual bool Initialize() = 0;
};

// include hooks here
#include "Hooks/Keyboard/KeymapHook.h"
#include "Hooks/MinecraftRenderContext/SetupAndRender.h"
#include "Hooks/MinecraftRenderContext/SchedulerRate.h"

void InitHooks() {
	// initialize hooks here
	static FuncHook* hooks[] = {
		// include hooks here
        &KeymapHook::Get(),
        &SetupAndRenderHook::Get(),
        &SchedulerRateHook::Get()
	};

    for (std::size_t i = 0; i < std::size(hooks); ++i)
    {
        if (not hooks[i]->Initialize())
        {
            //error handling
            LogMessage(MESSAGE_ERROR, "Failed to initialize hook %d", i);
        }
    }
}