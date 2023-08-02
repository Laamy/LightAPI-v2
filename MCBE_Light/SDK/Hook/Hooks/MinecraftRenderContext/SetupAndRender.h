#pragma once

void* __o__SetupAndRender;

#include "../../../GameCore/Rendering/UILayer.h"
#include "../../../GameCore/Rendering/RenderContext.h"

void SetupAndRenderDetour(ScreenView* screenview, RenderContext* ctx) {
	// render mc main layer
	CallFunc<void, ScreenView*, RenderContext*>(__o__SetupAndRender, screenview, ctx);

	// main frame render call if these pass
	if (UILayer::IsNot(screenview, {
			UILayer::Toast_ToastScreen,
			UILayer::Debug_DebugScreen,
		})) {

		for (Drawing_Rectangle* obj : drawing_rectangles) {
			if (!obj->visible)
				continue;

			if (obj->filled) {
				ctx->fillRectangle(Vector4<float>(obj->position, obj->size), obj->colour, obj->alpha);
			}
			else {
				ctx->drawRectangle(Vector4<float>(obj->position, obj->size), obj->colour, obj->alpha, obj->radius);
			}
		}
		
		// handle the waiting scripts (doing this here is cuz it'll crash or lag if we do it in the main loop, or any in general)

		// get script context
		Instances::ScriptContext* context = Instances::ScriptContext::Get();
		
		// execute resume job so scripts that called "wait" can be resumed once timeout is finished
		ResumeJob::Get()->ExecuteTask(context);

		// tell crash handler the game is still running
		//CrashJob::Get()->UpdateTask();

		// handle queued scripts do scripts one by one to avoid crashing
		if (!LuauHelper::QueuedScripts.empty()) {
			// get the top script then pop it off the queue
			Instances::ScriptInstance script = LuauHelper::QueuedScripts.front();
			LuauHelper::QueuedScripts.pop();

			// compile load then call the luau bytecode
			LuauHelper::ExecuteLuau(script.source.c_str(), script.chunkname.c_str(), LuauHelper::Security::DefaultScript);
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