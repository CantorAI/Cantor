#pragma once
#include "singleton.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include "Locker.h"
#include "frame.h"

class UIDHashFunction 
{
public:

	// We use predefined hash functions of unsigned long long
	// and define our hash function as XOR of the
	// hash values.
	size_t operator()(const UID& p) const
	{
		return (std::hash<unsigned long long> ()(p.l)) ^
			(std::hash<unsigned long long>()(p.h));
	}
};
class CantorHost;
struct NodeInfo
{
	UID id;
	std::string name;
	void* pSession = nullptr;
	std::unordered_map<std::string, unsigned long long> m_resMap;
	//key-value for resources such as GPU:4,CPU:16,CUDA:True(1),...
};
class NodeManager :
	public Singleton<NodeManager>
{
public:
	NodeManager();
	~NodeManager();
	void SetHost(CantorHost* p)
	{
		mHost = p;
	}
	void AddNode(std::string name, UID id,void* pSession);
	void RemoveNode(std::string name);
	void RemoveNode(UID id);
	bool QueryNodeId(std::string name, UID& uid);
	std::string QueryNodeName(UID id);
	void* QueryNodeSession(UID id);
private:
	CantorHost* mHost;
	Locker mLockNodes;
	std::unordered_map<std::string, UID> mMapNameToId;
	std::unordered_map<UID, NodeInfo, UIDHashFunction> mNodeMap;
};