#pragma once

class JobBase {
public:
	virtual bool ExecuteTask() = 0;
};

#include "Jobs/ResumeJob.h"