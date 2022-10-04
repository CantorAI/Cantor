#pragma once
#include "singleton.h"
class WorkerMode :
	public Singleton<WorkerMode>
{
public:
	void Loop();
};