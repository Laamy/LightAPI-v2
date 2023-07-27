#pragma once

void* __o__SetupAndRender;

#include "../../../GameCore/UILayer.h"

void SetupAndRenderDetour(ScreenView* screenview, uintptr_t mcRenderCtx) {
	// render mc main layer
	CallFunc<void, ScreenView*, uintptr_t>(__o__SetupAndRender, screenview, mcRenderCtx);

	// main frame render call if these pass
	if (UILayer::IsNot(screenview, {
			UILayer::Toast_ToastScreen,
			UILayer::Debug_DebugScreen,
		})) {
		Instances::ScriptContext* context = Instances::ScriptContext::Get();

		for (auto timeout : context->yieldThreads) {
			lua_State* thread = timeout.first;
			Instances::TimeoutInstance* timeoutInst = timeout.second;

			if (LuauHelper::GetTime() >= timeoutInst->end) {
				context->yieldThreads.erase(thread);
				lua_resume(thread, 0, 0);
			}
		}
	}
}

class SetupAndRenderHook : public FuncHook {
public:
	bool Initialize() override {
		uintptr_t setupAndRenderAddr = findOffset(xorstr_("48 8B C4 48 89 58 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 ? B8 0F 29 ? A8 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B F2 48 89 54 24 ? 4C"), xorstr_("SetupAndRender"));

		if (not HookFunction((void*)setupAndRenderAddr, &SetupAndRenderDetour, &__o__SetupAndRender))
			return false;

		return true;
	}

	static SetupAndRenderHook& Get() {
		static SetupAndRenderHook single;
		return single;
	}
};