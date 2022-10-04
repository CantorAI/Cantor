#include "NetworkManager.h"
#include "srvthread.h"
#include "clientthread.h"
#include "cantor_host.h"
#include "NodeManager.h"
#include "tmsession.h"
#include "log.h"

NetworkManager::NetworkManager()
{
}

NetworkManager::~NetworkManager()
{
}

bool NetworkManager::StartServer(int port, int maxConn)
{
	bool Opened = false;
	m_lockServers.Lock();
	for (auto srv : m_Servers)
	{
		if (srv->Port() == port)
		{
			Opened = true;
			break;
		}
	}
	m_lockServers.Unlock();
	if (Opened)
	{
		return true;
	}

	SrvThread* pSrv = new SrvThread();
	pSrv->Set(port, maxConn);
	pSrv->Init(mHost);
	pSrv->Start();
	m_lockServers.Lock();
	m_Servers.push_back(pSrv);
	m_lockServers.Unlock();
	return true;
}

bool NetworkManager::StopServer(int port)
{
	return false;
}

bool NetworkManager::Connect(std::string strSrv, int port)
{
	ClientThread* pClient = new ClientThread();
	pClient->Init(mHost, strSrv, port);
	pClient->Start();
	m_lockClients.Lock();
	m_Clients.push_back(pClient);
	m_lockClients.Unlock();
	return true;
}
void NetworkManager::RemoveClient(ClientThread* pClient)
{
	m_lockClients.Lock();
	auto it = m_Clients.begin();
	while (it != m_Clients.end())
	{
		if (*it == pClient)
		{
			it = m_Clients.erase(it);
			break;
		}
	}
	m_lockClients.Unlock();
}
bool NetworkManager::Disconnect()
{
	return false;
}

void NetworkManager::AddSession(TMSession* pSession)
{
	m_lockSessions.Lock();
	m_Sessions.push_back(pSession);
	m_lockSessions.Unlock();
}
void NetworkManager::RemoveSession(TMSession* pSession)
{
	auto strIP = pSession->GetAddr();
	auto port = pSession->GetPort();
	LOG << "Cantor:Session(IP:" << strIP << ",Port:" << port << ") diconnected";
	m_lockSessions.Lock();
	auto it = m_Sessions.begin();
	while (it != m_Sessions.end())
	{
		if (*it == pSession)
		{
			it = m_Sessions.erase(it);
			break;
		}
	}
	m_lockSessions.Unlock();
}
bool NetworkManager::SendFrameToNode(UID& nodeId, DataFrame& frm, int priority)
{
	bool bOK = false;
	void* pSessionPtr = NodeManager::I().QueryNodeSession(nodeId);
	if (pSessionPtr)
	{
		TMSession* pTMSession = (TMSession*)pSessionPtr;
		//TODO:check if this session is still valid
		bOK = pTMSession->SendFrame(frm, priority);
	}
	return bOK;
}

bool NetworkManager::SendFrameToAllNode(DataFrame& frm, int priority)
{
	m_lockSessions.Lock();
	for (auto p : m_Sessions)
	{
		p->SendFrame(frm, priority);
	}
	m_lockSessions.Unlock();
	return true;
}
bool NetworkManager::SendFrameToAllNode(DataFrame& frm, std::vector<UID>& exclude_uids, int priority)
{
	m_lockSessions.Lock();
	for (auto p : m_Sessions)
	{
		auto id = p->GetAntoherSideNodeId();
		bool bInExclude = false;
		for (auto& uid : exclude_uids)
		{
			if (id == uid)
			{
				bInExclude = true;
				break;
			}
		}
		if (bInExclude)
		{
			continue;
		}
		p->SendFrame(frm, priority);
	}
	m_lockSessions.Unlock();
	return true;
}