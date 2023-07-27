#pragma once

class ResumeJob : public JobBase {
public:
	bool ExecuteTask(Instances::ScriptContext* context) {
		for (auto timeout : context->yieldThreads) {
			lua_State* thread = timeout.first;
			Instances::TimeoutInstance* timeoutInst = timeout.second;

			if (LuauHelper::GetTime() >= timeoutInst->end) {
				context->ResumeThread(thread);
			}
		}

		return true;
	}

	// singlton
	static ResumeJob* Get() {
		static ResumeJob* instance;
		return instance;
	}
};