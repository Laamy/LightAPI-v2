#pragma once

void* __o__SchedulerRate;

uintptr_t SchedulerRateDetour(SchedulerRate* _this) {

	Game::Scheduler = _this;

	return CallFunc<uintptr_t, SchedulerRate*>(__o__SchedulerRate, _this);
}

class SchedulerRateHook : public FuncHook {
public:
	bool Initialize() override {
		uintptr_t schedulerRateAddr = findOffset(xorstr_("48 8B 41 08 48 8B 90 ? ? ? ? 48 85 D2 74 13 48 8B 42 08 48 8B CA 48 8B 90 ? ? ? ? 48 85 D2 75 ED 8B"), xorstr_("SchedulerRate"));

		if (not HookFunction((void*)schedulerRateAddr, &SchedulerRateDetour, &__o__SchedulerRate))
			return false;

		return true;
	}

	static SchedulerRateHook& Get() {
		static SchedulerRateHook single;
		return single;
	}
};