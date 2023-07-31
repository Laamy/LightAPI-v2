#pragma once

namespace Game {
	static std::map<int, bool> Keymap;

	SchedulerRate* Scheduler = nullptr;

	enum GameEvent {
		NONE,
		Update,
		KeyDown,
		KeyUp,
		KeyPress
	};
}