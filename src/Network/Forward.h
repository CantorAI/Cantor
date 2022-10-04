#pragma once

#include "singleton.h"
#include <vector>
#include <string>
#include "Locker.h"
#include "frame.h"
#include "gthread.h"
#include "cantor_host.h"

class Forwarder :
	public GThread,
	public Singleton<Forwarder>
{
public:
	Forwarder();
	~Forwarder();

	void Init(CantorHost* pHost)
	{
		mHost = pHost;
	}
private:
	// Inherited via GThread
	virtual void run() override;

	bool CreateDVWhenNodeIDSet(DataViewID& dvId);
	bool mRun = true;
	CantorHost* mHost = nullptr;
};