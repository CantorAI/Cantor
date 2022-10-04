#pragma once
#include "singleton.h"
#include <vector>
#include <string>
#include "Locker.h"
#include "frame.h"
#include "xpackage.h"
#include "xlang.h"

class CantorHost;
class SrvThread;
class ClientThread;
class TMSession;
class NetworkManager:
	public Singleton<NetworkManager>
{
public:
	NetworkManager();
	~NetworkManager();
	BEGIN_PACKAGE(NetworkManager)
		ADD_EVENT(OnNewSession)
		ADD_EVENT(OnConnected)
		ADD_FUNC(2,"StartServer", &NetworkManager::StartServer)
		ADD_FUNC(1,"StopServer", &NetworkManager::StopServer)
		ADD_FUNC(2,"Connect", &NetworkManager::Connect)
		ADD_FUNC(0,"Disconnect", &NetworkManager::Disconnect)
	END_PACKAGE

	void SetHost(CantorHost* pHost)
	{
		mHost = pHost;
	}
	bool StartServer(int port=1973, int maxConn = 30);
	bool StopServer(int port);

	bool Connect(std::string strSrv, int port);
	bool Disconnect();
	void AddSession(TMSession* pSession);
	void RemoveSession(TMSession* pSession);
	bool SendFrameToNode(UID& nodeId, DataFrame& frm, int priority=0);
	bool SendFrameToAllNode(DataFrame& frm, int priority=0);
	bool SendFrameToAllNode(DataFrame& frm, std::vector<UID>& exclude_uids,int priority = 0);
	void RemoveClient(ClientThread* pClient);
private:
	CantorHost* mHost = nullptr;
	Locker m_lockServers;
	std::vector<SrvThread*> m_Servers;
	Locker m_lockClients;
	std::vector<ClientThread*> m_Clients;
	Locker m_lockSessions;
	std::vector<TMSession*> m_Sessions;
};