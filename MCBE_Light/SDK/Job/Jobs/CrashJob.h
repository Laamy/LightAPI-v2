#pragma once

class CrashJob : public JobBase {
public:
	double lastFrame = LuauHelper::GetTime();
	bool hasInit = false;

	bool ExecuteTask() {
		if (LuauHelper::GetTime() - 5 >= lastFrame)
		{
			// crash has possibly happened so lets handle it
			if (!hasInit) {
				// game hasn't even started yet so lets ignore this "crash" for now as its probably not
				return true;
			}

			// alert the task executor that we have crashed
			std::cout << "Crash detected, exiting to avoid zombie process.." << std::endl;
			Sleep(3);
			ExitProcess(0);
			return false;
		}

		return true;
	}

	void UpdateTask() {
		lastFrame = LuauHelper::GetTime();

		// game rendered its first frame so lets inititalize the crash handler
		if (!hasInit) {
			hasInit = true;
		}
	}

	// singlton
	static CrashJob* Get() {
		static CrashJob* instance;
		return instance;
	}
};

int counter = 0;

// thread to handle possible game crashes
void CrashThread() {
	while (true) {
		CrashJob::Get()->ExecuteTask();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}