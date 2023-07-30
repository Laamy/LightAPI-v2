#pragma once

class ConsoleInputJob : public JobBase {
public:
	bool ExecuteTask(Instances::ScriptContext* context, std::string input) {
		std::lock_guard<std::mutex> lock(context->inputThreadsMutex);

		if (!context->inputThreads.empty()) {
			// get first thread in job queue
			Instances::InputInstance* instance = context->inputThreads.front();

			// pop first thread in job queue
			context->inputThreads.pop();

			// push input to thread
			lua_pushstring(instance->thread, input.c_str());

			// resume thread execution with 1 argument and no states called it so return 0 too
			int status = lua_resume(instance->thread, 0, 1);

			if (status != LUA_OK && status != LUA_YIELD) {
				const char* error = lua_tostring(instance->thread, -1);
				lua_pop(instance->thread, 1);

				LogMessage(MESSAGE_ERROR, error);

				return false;
			}
		}

		return true;
	}

	// singleton
	static ConsoleInputJob* Get() {
		static ConsoleInputJob* instance;
		return instance;
	}
};

// had to create a new thread for the input job else script queue would not be updated
void InputThread() {
	while (true) {
		Instances::ScriptContext* context = Instances::ScriptContext::Get();

		if (!context->inputThreads.empty()) {
			std::string input;
			std::cin >> input;

			ConsoleInputJob::Get()->ExecuteTask(context, input);
		}
	}
}