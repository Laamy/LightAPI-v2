#pragma once

class ConsoleInputJob : public JobBase {
public:
	bool ExecuteTask(Instances::ScriptContext* context, std::string input) {
		if (!context->inputThreads.empty()) {
			// erase from input threads map
			Instances::InputInstance* instance = context->inputThreads.front();
			context->inputThreads.pop();

			lua_pushstring(instance->thread, input.c_str());
			//lua_pushstring(instance->thread, "test");

			// resume thread execution
			lua_resume(instance->thread, 0, 1);//LuauHelper::GameState

			lua_pop(instance->thread, 1);
		}

		return true;
	}

	// singlton
	static ConsoleInputJob* Get() {
		static ConsoleInputJob* instance;
		return instance;
	}
};