#include "Forward.h"
#include "CantorHost.h"
#include "gxydef.h"
#include "log.h"
#include "utility.h"
#include "NetworkManager.h"

Forwarder::Forwarder()
{
}

Forwarder::~Forwarder()
{
}

void Forwarder::run()
{
	DataViewID	dvForward =0;
	while (mRun)
	{
		if (dvForward == 0)
		{//NodeID will be set if nodeId is inside database for this node
			if (!CreateDVWhenNodeIDSet(dvForward))
			{
				MS_SLEEP(100);
				continue;
			}
		}
		DataFrame frm;
		if (mHost->PopFrame(dvForward, 0, -1, frm))
		{
			//if srcAddr is empty,fill it with this node
			if (frm.head->srcAddr == UID())
			{
				frm.head->srcAddr = CantorHostImpl::I().GetNodeId();
			}
			LOG << "Forwarder,got frame:" << frm.head->type<<",DestAddr:"
				<< UIDToString(frm.head->dstAddr);
			NetworkManager::I().SendFrameToNode(frm.head->dstAddr, frm,2);
		}
	}
}

bool Forwarder::CreateDVWhenNodeIDSet(DataViewID& dvId)
{
	UID nodeId = ((CantorHostImpl*)mHost)->GetNodeId();
	if (nodeId.h == 0 && nodeId.l == 0)
	{
		return false;
	}
	std::string filter = "dstAddr !in ('','$NodeId:ThisNode$')";
	dvId = mHost->CreateDataView(filter);
	return (dvId!=0);
}
