#include "NodeManager.h"
#include "CantorHost.h"

NodeManager::NodeManager()
{
}

NodeManager::~NodeManager()
{
}

void NodeManager::AddNode(std::string name, UID id, void* pSession)
{
	mLockNodes.Lock();
	mMapNameToId.emplace(std::make_pair(name, id));
	mNodeMap.emplace(std::make_pair(id,NodeInfo{id,name,pSession}));
	mLockNodes.Unlock();
}

void NodeManager::RemoveNode(std::string name)
{
	mLockNodes.Lock();
	auto it = mMapNameToId.find(name);
	if (it != mMapNameToId.end())
	{
		UID& id = it->second;
		auto it2 = mNodeMap.find(id);
		if (it2 != mNodeMap.end())
		{
			mNodeMap.erase(it2);
		}
		mMapNameToId.erase(it);
		
	}
	mLockNodes.Unlock();
}

void NodeManager::RemoveNode(UID id)
{
	mLockNodes.Lock();
	auto it = mNodeMap.find(id);
	if (it != mNodeMap.end())
	{
		std::string& name = it->second.name;
		auto it2 = mMapNameToId.find(name);
		if (it2 != mMapNameToId.end())
		{
			mMapNameToId.erase(it2);
		}
		mNodeMap.erase(it);
	}
	mLockNodes.Unlock();
}

bool NodeManager::QueryNodeId(std::string name,UID& uid)
{
	bool bHave = false;
	if (name.size() == 0)
	{
		bHave = true;
		uid = UID();//NULL
	}
	else if (name == CantorHostImpl::I().GetNodeName())
	{
		bHave = true;
		uid = CantorHostImpl::I().GetNodeId();
	}
	else
	{
		mLockNodes.Lock();
		auto it = mMapNameToId.find(name);
		if (it != mMapNameToId.end())
		{
			uid = it->second;
			bHave = true;
		}
		mLockNodes.Unlock();
	}
	return bHave;
}

std::string NodeManager::QueryNodeName(UID id)
{
	std::string name;
	mLockNodes.Lock();
	auto it = mNodeMap.find(id);
	if (it != mNodeMap.end())
	{
		name = it->second.name;
	}
	mLockNodes.Unlock();
	return name;
}
void* NodeManager::QueryNodeSession(UID id)
{
	void* pSession = nullptr;
	mLockNodes.Lock();
	auto it = mNodeMap.find(id);
	if (it != mNodeMap.end())
	{
		pSession = it->second.pSession;
	}
	mLockNodes.Unlock();
	return pSession;
}
