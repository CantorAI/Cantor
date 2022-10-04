#pragma once

#include "singleton.h"
#include <vector>
#include <string>
#include "Locker.h"
#include "gthread.h"
#include "frame.h"

class CantorHost;
class RegistryServer :
	public GThread,
	public Singleton<RegistryServer>
{
public:
	void SetHost(CantorHost* pHost)
	{
		mHost = pHost;
	}
private:
	CantorHost* mHost = nullptr;
	bool mRun = true;
	// Inherited via GThread
	virtual void run() override;
};
