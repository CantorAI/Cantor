#include "RegistryClient.h"
#include "clientthread.h"
#include "tmsession.h"
#include "service_def.h"

RegistryClient::RegistryClient()
{
}

RegistryClient::~RegistryClient()
{
}

//Thread for publishing to registry
//Connect to Server one by one, when connection is established,
//send node register frame and receive command frames
//then disconect it

void RegistryClient::ProcessRegistryAck(DataFrame& frm)
{
}

void RegistryClient::run()
{
	struct CallInfo
	{
		ClientThread* pClient;
		UID Srv_NodeId;
	};
	auto SessionCB = [](void* pContext, TMSession* pSession,
		std::string& srv_nodeName, UID& srv_nodeId)
	{
		DataFrame frm;
		frm.head->type = (FrameType)DataFrameType::Framework_RegisterToRegistry;
		frm.head->dstAddr = srv_nodeId;

		pSession->SendFrame(frm, 0);
	};
	const int buf_Len = 1000;
	char szFilter[buf_Len];
	SPRINTF(szFilter, 1000,
		"type == %d && dstAddr in ('','$NodeId:ThisNode$')",
		(int)DataFrameType::Framework_RegisterToRegistry_Ack);
	std::string strFilter(szFilter);
	DataViewID dvId = mHost->CreateDataView(strFilter);
	int sleep_time = 1000;
	while (mRun)
	{
		std::vector<CallInfo*> callList;
		mRegSrvLock.Lock();
		for (auto& srvInfo : m_regSrvs)
		{
			CallInfo* pCallInfo = new CallInfo();
			ClientThread* pClient = new ClientThread();
			pCallInfo->pClient = pClient;
			pClient->SetShortSession(true);
			pClient->SetOnConnectedCall(SessionCB, (void*)pCallInfo);
			pClient->Init(mHost, srvInfo.srv_address, srvInfo.port);
			pClient->Start();
			callList.push_back(pCallInfo);
		}
		mRegSrvLock.Unlock();
		//TODO: if registry server is down, need to timeout client
		//in the loop below
		while (callList.size() > 0)
		{
			DataFrame frmAck;
			if (mHost->PopFrame(dvId, 0, 1000, frmAck))
			{
				for (auto it = callList.begin(); it != callList.end(); it++)
				{
					CallInfo* pCallInfo = *it;
					if (frmAck.head->srcAddr == pCallInfo->Srv_NodeId)
					{
						if (pCallInfo->pClient)
						{
							pCallInfo->pClient->Close();
							//pClient will be deleted by ClientThread's thread
						}
						ProcessRegistryAck(frmAck);
						callList.erase(it);
						delete pCallInfo;
						break;
					}
				}//end for
			}//end if
		}//end while
		MS_SLEEP(sleep_time);
	}
	mHost->RemoveDataView(dvId);
}
