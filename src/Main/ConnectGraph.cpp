#include "ConnectGraph.h"
#include "utility.h"

ConnectGraph::ConnectGraph()
{
}

ConnectGraph::~ConnectGraph()
{
}

void ConnectGraph::RemoveEdge(UID nodeid1, UID nodeid2)
{
	mEdgeLock.Lock();
	for (auto it = mEdges.begin();it != mEdges.end(); it++)
	{
		if ((it->startNodeId == nodeid1 && it->endNodeId == nodeid2) ||
			(it->startNodeId == nodeid2 && it->endNodeId == nodeid1))
		{
			mEdges.erase(it);
		}
		else
		{
			++it;
		}
	}
	mEdgeLock.Unlock();
}

void ConnectGraph::ChangeEdgeStatus(UID nodeid1, UID nodeid2, bool isOnline)
{
	mEdgeLock.Lock();
	for (auto it = mEdges.begin(); it != mEdges.end(); it++)
	{
		if (it->startNodeId == nodeid1 && it->endNodeId == nodeid2)
		{
			it->isOnline = isOnline;
			break;
		}
	}
	mEdgeLock.Unlock();
}

ConnType ConnectGraph::FindConnection(UID nodeid1, UID nodeid2)
{
	ConnType ct = ConnType::NoConnection;
	mEdgeLock.Lock();
	for (auto it = mEdges.begin(); it != mEdges.end(); it++)
	{
		if (it->startNodeId == nodeid1 && it->endNodeId == nodeid2)
		{
			ct = ConnType::Connected;
			break;
		}
		else if(it->startNodeId == nodeid2 && it->endNodeId == nodeid1)
		{
			ct = ConnType::Reverse;
			break;
		}
	}
	mEdgeLock.Unlock();
	return ct;
}
