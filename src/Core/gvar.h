#pragma once

#include "singleton.h"
#include "xpackage.h"
#include "xlang.h"
#include "Locker.h"
#include "CantorHost.h"
#include <unordered_map>
#include <vector>
#include "gthread.h"
#include "CantorWait.h"

class GVar;
struct TransforFrameInfo
{
	UID srcId;
	TimeStamp ts;
};
#define MAX_TANSFER_FRM_CACHE_CNT 100
class GVarManager :
	public GThread, 
	public Singleton<GVarManager>
{
	Locker m_lock;
	std::unordered_map<std::string, GVar*> m_gvarmap;
	Locker m_lockTransfer;
	std::vector<TransforFrameInfo> m_TransferFrameInfos;
	bool HaveBeenTransfered(UID& src, TimeStamp& ts)
	{
		bool bTranfer = false;
		m_lockTransfer.Lock();
		for (auto& i : m_TransferFrameInfos)
		{
			if (i.srcId == src && i.ts == ts)
			{
				bTranfer = true;
				break;
			}
		}
		m_lockTransfer.Unlock();
		return bTranfer;
	}
	void PushTransferFrameInfo(UID& src, TimeStamp& ts)
	{
		m_lockTransfer.Lock();
		if (m_TransferFrameInfos.size() > MAX_TANSFER_FRM_CACHE_CNT)
		{
			m_TransferFrameInfos.erase(m_TransferFrameInfos.begin());
		}
		m_TransferFrameInfos.push_back({ src,ts });
		m_lockTransfer.Unlock();

	}
	bool m_run = true;
	// Inherited via GThread
	virtual void run() override;
public:
	GVarManager()
	{
	}
	void Add(std::string& key, GVar* pVar)
	{
		m_lock.Lock();
		auto it = m_gvarmap.find(key);
		if (it != m_gvarmap.end())
		{
			delete it->second;
		}
		m_gvarmap.emplace(std::make_pair(key,pVar));
		m_lock.Unlock();
	}
	GVar* Query(std::string name)
	{
		GVar* pVar = nullptr;
		m_lock.Lock();
		auto it = m_gvarmap.find(name);
		if (it != m_gvarmap.end())
		{
			pVar = it->second;
		}
		m_lock.Unlock();
		return pVar;
	}
};
enum class GVar_Op
{
	QueryOwner,
	SetValue,
	GetValue,
	Subscribe,
	Unsubscribe,
	ValueChanged,
};
class GVar
{
	std::string m_key;
	bool m_valIsValid = false;
	X::Value m_val;
	UID m_ownerNodeId;
	bool m_needSendUpdateFrame = false;
	bool m_haveWait = false;
	bool m_HasEventHandlers = false;
	CantorWait* m_wait = nullptr;
	std::vector<UID> m_subscribers;
	Locker m_lock;
public:
	GVar(std::string key)
	{
		RegisterAPIS();
		m_key = key;
		m_wait = new CantorWait();
		GVarManager::I().Add(key, this);

		m_Apis.GetEvent(0)->SetChangeHandler([this](bool AddOrRemove,int cnt) 
			{
				if (cnt > 0)
				{
					if (m_ownerNodeId.IsNull())
					{
						m_HasEventHandlers = true;
						Query();
					}
					else
					{
						PostSubscribeFrame();
					}
				}
				else
				{
					PostUnsubscribeFrame();
				}
			});
	}
	~GVar()
	{
		if (m_wait)
		{
			delete m_wait;
		}
	}
	void AddSub(UID& id)
	{
		m_lock.Lock();
		bool bFind = false;
		for (auto& sid : m_subscribers)
		{
			if (sid == id)
			{
				bFind = true;
				break;
			}
		}
		if (!bFind)
		{
			m_subscribers.push_back(id);
		}
		m_lock.Unlock();
	}
	void RemoveSub(UID& id)
	{
		m_lock.Lock();
		for (auto it = m_subscribers.begin(); it != m_subscribers.end(); it++)
		{
			if (*it == id)
			{
				m_subscribers.erase(it);
				break;
			}
			else
			{
				++it;
			}
		}
		m_lock.Unlock();
	}
	void SetValue(X::Value& v) 
	{ 
		m_valIsValid = true;
		m_val = v;
		X::ARGS params{ v };
		X::KWARGS kwParams;
		APISET().Fire(0, params, kwParams);
		NotiSubs();
	}
	UID GetOwner() { return m_ownerNodeId; }
	void SetOwner(UID& id)
	{
		m_ownerNodeId = id;
		if (m_needSendUpdateFrame)
		{
			m_needSendUpdateFrame = false;
			PostUpdateFrame();
		}
		if (m_HasEventHandlers)
		{
			PostSubscribeFrame();
			m_HasEventHandlers = false;
		}
	}
	bool HaveWait()
	{
		return m_haveWait;
	}
	void ReleaseWait() { m_haveWait = false; m_wait->Release(); }
	X::Value& GetValue() { return m_val; }
	bool HaveOwnerId()
	{
		return !m_ownerNodeId.IsNull();
	}
	BEGIN_PACKAGE(GVar)
		ADD_EVENT(changed)
		ADD_FUNC(0, "create", &GVar::Create)
		ADD_FUNC(1, "set", &GVar::Set)
		ADD_FUNC(0, "get", &GVar::Get)
		ADD_FUNC(0, "delete", &GVar::Delete)
	END_PACKAGE
	bool Create()
	{
		auto myNode = CantorHostImpl::I().GetNodeId();
		if (m_ownerNodeId.IsNull())
		{
			//make this gvar owned by this node
			m_ownerNodeId = myNode;
			return true;
		}
		else
		{
			return myNode == m_ownerNodeId;//same node with second call will be OK
		}
	}
	void Query();
	bool Set(X::Value& var);
	void PostUpdateFrame();
	void PostSubscribeFrame();
	void PostUnsubscribeFrame();
	void NotiSubs();
	X::Value Get();
	bool Delete()
	{
		return true;
	}
};