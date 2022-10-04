#include "CantorHost.h"
#include "log.h"
#include "DataView.h"
#include "KVCache.h"
#include "utility.h"
#include "workermanager.h"
#include "TaskManager.h"
#include "DataFrameWrapper.h"

CantorHostImpl::CantorHostImpl()
{
}

CantorHostImpl::~CantorHostImpl()
{
}

bool CantorHostImpl::Start()
{

	return true;
}

void CantorHostImpl::Shutdown()
{
	LOG << "Server::Shutdown";
}
std::string CantorHostImpl::Generate_UID()
{
    return DC::TaskManager::I().GenGuid();
}
void CantorHostImpl::Print(std::string strInfo)
{
}

void CantorHostImpl::Print(std::wstring strInfo)
{
}

DataViewID CantorHostImpl::CreateDataView(std::string& filter)
{
    DataView* pDV = DataView::Create(filter);
    if (pDV == nullptr)
    {
        return (DataViewID)0;
    }
    DataViewID id = (DataViewID)pDV;
    mDataViewLock.Lock();
    mDVMap.emplace(std::make_pair(id,pDV));
    mDataViewLock.Unlock();
    return id;
}

void CantorHostImpl::ReleaseDataViewWaits(DataViewID id)
{
    DataView* pDV = nullptr;
    mDataViewLock.Lock();
    auto it = mDVMap.find(id);
    if (it != mDVMap.end())
    {
        pDV = it->second;
        mDVMap.erase(it);
    }
    mDataViewLock.Unlock();
    if (pDV)
    {
        pDV->ReleaseWaits();
    }
}
X::Value CantorHostImpl::PopFrameAPI(DataViewID dvId, TimeStamp ts, int timeout_ms)
{
    X::XPackageValue<DataFrameWrapper> valFrm;
    PopFrame(dvId, ts, timeout_ms, *valFrm);
    return valFrm;
}
bool CantorHostImpl::RemoveDataView(DataViewID id)
{
    bool bFind = false;
    mDataViewLock.Lock();
    auto it = mDVMap.find(id);
    if (it != mDVMap.end())
    {
        delete it->second;
        mDVMap.erase(it);
        bFind = true;
    }
    mDataViewLock.Unlock();
    return bFind;
}

bool CantorHostImpl::PushFrame(const DataFrame& frm)
{
    mDataViewLock.Lock();
    for (auto& it : mDVMap)
    {
        it.second->CheckAndInsert((DataFrame&)frm);
    }
    mDataViewLock.Unlock();
    return true;
}

bool CantorHostImpl::PopFrame(DataViewID dvId, TimeStamp ts, int timeout_ms, DataFrame& frm)
{
    bool bHave = false;
    DataView* pDV = nullptr;
    mDataViewLock.Lock();
    auto it = mDVMap.find(dvId);
    if (it != mDVMap.end())
    {
        pDV = it->second;
    }
    mDataViewLock.Unlock();
    if (pDV)
    {

        bHave = pDV->Pop(frm, ts, timeout_ms);
    }
    return bHave;
}

std::string CantorHostImpl::GetNodeName()
{
    CantorVar varNodeName;
    std::string tag(NODE_NAME_TAG);
    if (KVCache::I().Get(tag, varNodeName))
    {
        return varNodeName;
    }
    else
    {
        return "";
    }
}
UID CantorHostImpl::GetNodeId()
{
    UID uid;
    CantorVar varIdName;
    std::string tag(NODE_ID_TAG);
    if (KVCache::I().Get(tag, varIdName))
    {
        uid = UIDFromString(varIdName);
    }
    return uid;
}
