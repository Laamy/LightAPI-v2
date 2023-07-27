#pragma once

void* __o__keypress;

void KeypressDetour(int key, bool held) {
	Game::Keymap[key] = held;

	if (held) {
		std::stringstream ss;
		ss << "Game:CallHooks('keydown', " << key << ")";

		LuauHelper::ExecuteLuau(ss.str().c_str(),
			"KeymapScript", LuauHelper::Security::SystemScript);
	}

	if (!held) {
		std::stringstream ss;
		ss << "Game:CallHooks('keyup', " << key << ")";

		LuauHelper::ExecuteLuau(ss.str().c_str(),
			"KeymapScript", LuauHelper::Security::SystemScript);
	}

	return CallFunc<void, int, bool>(__o__keypress, key, held);
}

class KeymapHook : public FuncHook {
public:
	bool Initialize() override {
		uintptr_t keymapAddr = findOffset(xorstr_("48 83 EC 48 ? ? C1 4C 8D"), xorstr_("Keymap"));

		if (not HookFunction((void*)keymapAddr, &KeypressDetour, &__o__keypress))
			return false;

		return true;
	}

	static KeymapHook& Get() {
		static KeymapHook single;
		return single;
	}
};