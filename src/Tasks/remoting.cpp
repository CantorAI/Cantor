#include "remoting.h"
#include "log.h"
#include "frame.h"
#include "FramePack.h"
#include "gxydef.h"
#include "workermanager.h"
#include "CantorHost.h"
#include "NodeManager.h"
#include "service_def.h"

DC::RemotingMethodProcess::RemotingMethodProcess()
{
}
//moduleName:string,path:string,serverName:string
DC::RemotingMethodProcess::~RemotingMethodProcess()
{
}

void DC::RemotingMethodProcess::run()
{
    const int timeout_ms = 100;
    const int buf_Len = 1000;
    char szFilter[buf_Len];
    SPRINTF(szFilter, 1000,
        "type == %d && metadata[0] == 0 && dstAddr in ('','$NodeId:ThisNode$')",
        (int)DataFrameType::Framework_Command);
    std::string strFilter(szFilter);
    DataViewID dvId = CantorHostImpl::I().CreateDataView(strFilter);
    std::vector<DataFrame> waitlist;
    const int maxWaitFrameNum = 30;
    while (mRun)
    {
        DataFrame frm;
        bool bOK = CantorHostImpl::I().PopFrame(dvId,0,timeout_ms,frm);
        if (bOK)
        {
            waitlist.push_back(frm);
        }
        while (waitlist.size() > 0)
        {
            auto pWorker = DC::WorkerManager::I().FindOrCreateIdelWorker(true);
            if (pWorker)
            {
                DataFrame frm0 = *waitlist.begin();
                waitlist.erase(waitlist.begin());

                frm0.head->metadata[0] = pWorker->processId;
                //then this worker will pick up
                mHost->PushFrame(frm0);
                pWorker->DecRef();
            }
            else
            {
                break;
            }
        }
        if (waitlist.size() >= maxWaitFrameNum)
        {
            //TODO:notify back the caller,no worker available
            waitlist.clear();
        }
    }
}
