#include "RegistryServer.h"
#include "service_def.h"
#include "gxydef.h"
#include "CantorHost.h"
#include "log.h"
#include "port.h"
#if (!WIN32)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

void RegistryServer::run()
{
	const int buf_Len = 1000;
	char szFilter[buf_Len];
	SPRINTF(szFilter, 1000,
		"type == %d && dstAddr in ('','$NodeId:ThisNode$')",
		(int)DataFrameType::Framework_RegisterToRegistry);
	std::string strFilter(szFilter);
	DataViewID dvId = mHost->CreateDataView(strFilter);
	while (mRun)
	{
		DataFrame frm;
		if (mHost->PopFrame(dvId, 0, 1000, frm))
		{
			struct in_addr paddr;
			paddr.s_addr = frm.head->format[0];
			std::string strIP = inet_ntoa(paddr);
			LOG << "Received Registry From Client IP:" << strIP;
			DataFrame ackFrm;
			ackFrm.head->type = (FrameType)DataFrameType::Framework_RegisterToRegistry_Ack;
			ackFrm.head->dstAddr = frm.head->srcAddr;
			mHost->PushFrame(ackFrm);
		}
	}
	mHost->RemoveDataView(dvId);
}
