#pragma once
#include "singleton.h"
#include <vector>
#include "Locker.h"
#include "frame.h"

enum class ConnType
{
	NoConnection,
	Connected,
	Reverse,
};
struct ConnectGraphEdgeInfo
{
	UID startNodeId;
	UID endNodeId;
	bool isOnline = false;
};
class ConnectGraph :
	public Singleton<ConnectGraph>
{
public:
	ConnectGraph();
	~ConnectGraph();

	void AddEdge(UID nodeid1,UID nodeid2)
	{
		mEdgeLock.Lock();
		mEdges.push_back({ nodeid1, nodeid2,true });
		mEdgeLock.Unlock();
	}
	void RemoveEdge(UID nodeid1,UID nodeid2);
	void ChangeEdgeStatus(UID nodeid1, UID nodeid2,bool isOnline);
	ConnType FindConnection(UID nodeid1, UID nodeid2);
private:
	Locker mEdgeLock;
	std::vector <ConnectGraphEdgeInfo> mEdges;
};