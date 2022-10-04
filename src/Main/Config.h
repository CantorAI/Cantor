#pragma once

#include "singleton.h"

class CantorHost;
class Config :
	public Singleton<Config>
{
public:
	void SetHost(CantorHost* pHost)
	{
		mHost = pHost;
	}
	bool Load();
private:
	CantorHost* mHost = nullptr;
};
