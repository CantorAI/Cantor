#pragma once

#include "gxydef.h"

class CantorWait
{
public:
	CantorWait(bool autoReset=true);
	~CantorWait();
	bool Wait(int timeoutMS);
	void Release();
private:
	bool m_autoReset = true;
	PasWaitHandle m_waitHandle = nullptr;
};