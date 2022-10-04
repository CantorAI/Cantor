#pragma once

#include <unordered_map>
#include <vector>
#include "Locker.h"
#include "singleton.h"
#include <string>
#include "frame.h"
#include "NodeManager.h"
#include "xpackage.h"
#include <iostream>

class CantorHost;
namespace DC //Distributed Computing
{
 
    class TASK
    {
        X::Value m_originalObject;
        X::XPackageAPISet<TASK> m_Apis;
    public:
        X::XPackageAPISet<TASK>& APISET() { return m_Apis; }
        TASK()
        {
            m_Apis.AddVarFunc("run", &TASK::Run);
            m_Apis.Create(this);
        }
        void SetOriginalObject(X::Value& v)
        {
            m_originalObject = v;
        }
        void Run(X::XRuntime* rt, X::XObj* pContext,
            X::ARGS& params, X::KWARGS& kwParams, X::Value& retValue);
    public:
        std::string name;
        UID taskId;
        std::string uniqueId;//string format of taskId
    };
    enum class TaskInstanceResultStatus
    {
        NotReady =0,
        Ready =1,
        Error =-1
    };
    class TaskInstance
    {
        X::XPackageAPISet<TaskInstance> m_Apis;
    public:
        X::XPackageAPISet<TaskInstance>& APISET() { return m_Apis; }
        TaskInstance()
        {
            m_Apis.AddVarFunc("get", &TaskInstance::Get);
            m_Apis.Create(this);
        }
        void Get(X::XRuntime* rt, X::XObj* pContext,
            X::ARGS& params, X::KWARGS& kwParams, X::Value& retValue);
        ~TaskInstance()
        {
            if (content)
            {
                delete content;
            }
        }
        TASK* mTask;
        UID id;
        std::string uniqueId;
        char* content = nullptr;
        int contentSize =0;
    };

    class TaskManager :
        public Singleton<TaskManager>
    {
    public:
        TaskManager();
        ~TaskManager();
        BEGIN_PACKAGE(TaskManager)
        END_PACKAGE
        void SetHost(CantorHost* p)
        {
            mHost = p;
        }
        void RegisterTask(X::XRuntime* rt, X::XObj* pContext,
            X::ARGS& params, X::KWARGS& kwParams, X::Value& funcBytes,X::Value& retValue);
        std::string GenGuid();
        UID RegisterTask(std::string name, char* pContent, int size);
        UID SubmitTask(UID& taskId, char* pParamContent, int size);
        bool GetTaskForWorker(unsigned long workerProcessId, DataFrame& df);
        bool SetTaskStatusForWorker(DataFrame& df);
    private:
        X::Value mValRunner;
        Locker mTaskLock;
        std::unordered_map<std::string, TASK*> mTasks;
        std::unordered_map<UID, TASK*, UIDHashFunction> mTaskIDMap;
        Locker mInstanceLock;
        std::vector<TaskInstance*> mInstances;
        std::unordered_map<UID, TaskInstance*, UIDHashFunction> mInstanceMap;

        Locker mRunningLock;
        std::unordered_map<UID,TaskInstance*, UIDHashFunction> mRunnings;

        CantorHost* mHost = nullptr;
    };
}