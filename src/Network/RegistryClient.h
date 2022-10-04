#pragma once

#include "singleton.h"
#include <vector>
#include <string>
#include "Locker.h"
#include "gthread.h"
#include "frame.h"

class CantorHost;
class RegistryClient :
	public GThread,
	public Singleton<RegistryClient>
{
	struct RegistryServerInfo
	{
		std::string srv_address;
		int port;
	};
public:
	RegistryClient();
	~RegistryClient();
	void SetHost(CantorHost* pHost)
	{
		mHost = pHost;
	}
	void AddRegistryServer(std::string& address, int port)
	{
		mRegSrvLock.Lock();
		m_regSrvs.push_back(RegistryServerInfo{ address,port });
		mRegSrvLock.Unlock();
	}
private:
	CantorHost* mHost = nullptr;
	bool mRun = true;
	// Inherited via GThread
	virtual void run() override;

	Locker mRegSrvLock;
	std::vector<RegistryServerInfo> m_regSrvs;
	void ProcessRegistryAck(DataFrame& frm);
};