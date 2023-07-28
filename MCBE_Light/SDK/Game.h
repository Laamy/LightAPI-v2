#pragma once

namespace Game {
	static std::map<int, bool> Keymap;

	enum GameEvent {
		NONE,
		Update,
		KeyDown,
		KeyUp,
		KeyPress
	};
}