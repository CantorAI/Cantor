#pragma once
#include "singleton.h"
#include "xpackage.h"
#include "KVCache.h"
#include "NetworkManager.h"
#include "DataFrameWrapper.h"
#include "CantorHost.h"
#include "TaskManager.h"
#include "objectstore.h"
#include "gvar.h"

class CantorAPISet :
    public Singleton<CantorAPISet>
{
    X::XPackageAPISet<CantorAPISet> m_Apis;
public:
    X::XPackageAPISet<CantorAPISet>& APISET() { return m_Apis; }
public:
    CantorAPISet()
    {
        CantorHostImpl::I().RegisterAPIS();
        KVCache::I().RegisterAPIS();
        DC::TaskManager::I().RegisterAPIS();
        NetworkManager::I().RegisterAPIS();
        m_Apis.AddClass<0, CantorHostImpl>("Host", &CantorHostImpl::I());
        m_Apis.AddClass<0, KVCache>("KV", &KVCache::I());
        m_Apis.AddClass<0, NetworkManager>("Network", &NetworkManager::I());
        m_Apis.AddClass<0, DC::TaskManager>("TaskManager", &DC::TaskManager::I());
        m_Apis.AddClass<1, DataFrameWrapper>("DataFrame");
        m_Apis.AddClass<1, GVar>("gvar");
        m_Apis.AddVarFuncEx("Task", &CantorAPISet::Task);
        m_Apis.AddRawParamFunc("Set", &CantorAPISet::ObjectStore_Set);
        m_Apis.AddVarFunc("Get", &CantorAPISet::ObjectStore_Get);
        m_Apis.Create(this);
    }
    inline void Task(X::XRuntime* rt, X::XObj* pContext,
        X::ARGS& params, X::KWARGS& kwParams, X::Value& trailer,X::Value& retValue)
    {
        DC::TaskManager::I().RegisterTask(rt, pContext, params, kwParams, trailer,retValue);
    }
    inline void ObjectStore_Set(X::XRuntime* rt, X::XObj* pContext,
        X::ARGS& params, X::KWARGS& kwParams, X::Value& trailer, X::Value& retValue)
    {
        DC::StoreManager::I().SetValue(trailer, retValue);
    }
    inline X::Value ObjectStore_Get(X::XRuntime* rt, X::XObj* pContext,
        X::ARGS& params, X::KWARGS& kwParams,X::Value& retValue)
    {
        int timeout = -1;
        std::string strObjRef;
        if (params.size() > 0)
        {
            strObjRef = (std::string)params[0];
        }
        if (params.size() > 1)
        {
            timeout = (int)params[1];
        }
        return DC::StoreManager::I().GetValue(strObjRef,timeout);
    }
};
