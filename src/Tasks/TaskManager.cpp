#include "TaskManager.h"
#include "utility.h"
#include "dbop.h"
#include "dbmgr.h"
#include "tensor.h"
#include "objectstore.h"
#include "workermanager.h"
#include "CantorHost.h"
#if !(WIN32)
#include <uuid/uuid.h>
#endif
#include "utility.h"
#include <iostream>


DC::TaskManager::TaskManager()
{
}

DC::TaskManager::~TaskManager()
{
}

std::string DC::TaskManager::GenGuid()
{
	std::string strGuid;
#if (WIN32)
	GUID gid;
	CoCreateGuid(&gid);
	char szGuid[128];
	sprintf(szGuid, "%08X%04X%04X%X%X%X%X%X%X%X%X",
		gid.Data1, gid.Data2, gid.Data3,
		gid.Data4[0], gid.Data4[1], gid.Data4[2], gid.Data4[3],
		gid.Data4[4], gid.Data4[5], gid.Data4[6], gid.Data4[7]);
	//StringFromGUID2(gid, wGuid, 128);
	//std::wstring wstrGuid(wGuid);
	strGuid = szGuid;// ws2s(wstrGuid);
#else
	uuid_t uuid;
	uuid_generate_time_safe(uuid);
	char szGuid[128];
	uuid_unparse(uuid, szGuid);

	strGuid = szGuid;
#endif
	return strGuid;
}
UID DC::TaskManager::RegisterTask(
	std::string name, char* pContent, int size)
{
	UID taskId;
	mTaskLock.Lock();
	auto it = mTasks.find(name);
	if (it == mTasks.end())
	{
		TASK* pTaskInfo = new TASK();
		pTaskInfo->name = name;
		pTaskInfo->uniqueId = GenGuid();
		taskId = pTaskInfo->taskId = UIDFromString(pTaskInfo->uniqueId);
		mTasks.emplace(std::make_pair(name, pTaskInfo));
		mTaskIDMap.emplace(std::make_pair(taskId, pTaskInfo));
		DatabaseManager::I().BeginTransaction();
		DBStatement stat("insert into TaskCache Values(?,?,?)");
		std::wstring wstrGuid = s2ws(pTaskInfo->uniqueId);
		stat.bindtext(1, wstrGuid);
		stat.bindblob(2, pContent,size);
		stat.bindtext(3, s2ws(name));
		stat.step();
		DatabaseManager::I().EndTransaction();
	}
	else
	{
		TASK* pTaskInfo = it->second;
		taskId = pTaskInfo->taskId;
		if (size > 0)
		{
			DatabaseManager::I().BeginTransaction();
			DBStatement stat("update TaskCache set TaskContent=? where TaskId=?");
			std::wstring wstrGuid = s2ws(pTaskInfo->uniqueId);
			stat.bindblob(1, pContent, size);
			stat.bindtext(2, wstrGuid);
			stat.step();
			DatabaseManager::I().EndTransaction();
		}
	}
	mTaskLock.Unlock();
	return taskId;
}
UID DC::TaskManager::SubmitTask(
	UID& taskId, char* pParamContent, int size)
{
	bool bOK = false;
	UID instanceId;
	mTaskLock.Lock();
	auto it = mTaskIDMap.find(taskId);
	if (it != mTaskIDMap.end())
	{
		TASK* pTaskInfo = it->second;
		TaskInstance* pInst = new TaskInstance();
		pInst->mTask = pTaskInfo;
		pInst->uniqueId = GenGuid();
		pInst->id = instanceId = UIDFromString(pInst->uniqueId);
		pInst->content = new char[size];
		memcpy(pInst->content, pParamContent, size);
		pInst->contentSize = size;
		mInstanceLock.Lock();
		mInstances.push_back(pInst);
		mInstanceMap.emplace(std::make_pair(instanceId, pInst));
		mInstanceLock.Unlock();
		bOK = true;
	}
	mTaskLock.Unlock();
	if (bOK)
	{
		if (!DC::StoreManager::I().Create(instanceId))
		{
			instanceId =UID();
		}
	}
	return instanceId;
}

bool DC::TaskManager::GetTaskForWorker(unsigned long workerProcessId, DataFrame& df)
{
	Worker* pWorker = WorkerManager::I().QueryOrAddWorker(workerProcessId);
	bool bHaveTask = false;
	mInstanceLock.Lock();
	if (mInstances.size() > 0)
	{
		TaskInstance* pInst = mInstances[0];
		mInstances.erase(mInstances.begin());
		auto itMap = mInstanceMap.find(pInst->id);
		if (itMap != mInstanceMap.end())
		{
			mInstanceMap.erase(itMap);
		}

		TensorFrame ts;

		TASK* pTaskInfo = pInst->mTask;
		std::wstring funcKey;
		if (pTaskInfo)
		{
			DBStatement stat("select TaskContent,TaskName from TaskCache where TaskId=?");
			std::wstring wstrGuid = s2ws(pTaskInfo->uniqueId);
			stat.bindtext(1, wstrGuid);
			if (stat.step() == DBState::Row)
			{
				stat.getValue(1, funcKey);

				int blobSize = stat.getBlobSize(0);
				char* pData = (char*)stat.getBlobValue(0);
				ts.Add("Task", { blobSize }, sizeof(char), TENSOR_DATA_TYPES::BYTE);
				auto tsTask = ts["Task"];
				memcpy(tsTask.D<char>(), pData, blobSize);
			}
		}
		std::string funcName = ws2s(funcKey);
		int pos = funcName.find('.');
		if (pos > 0)
		{
			funcName = funcName.substr(pos + 1);
		}

		ts.Add("TaskInstance", { pInst->contentSize }, sizeof(char),TENSOR_DATA_TYPES::BYTE);
		auto tsInst = ts["TaskInstance"];
		memcpy(tsInst.D<char>(), pInst->content, pInst->contentSize);

		ts.Add("TaskInstanceID", { (int)pInst->uniqueId.length()+1 },
			sizeof(char), TENSOR_DATA_TYPES::BYTE);
		auto tsInstId = ts["TaskInstanceID"];
		memcpy(tsInstId.D<char>(), pInst->uniqueId.c_str(), pInst->uniqueId.length() + 1);

		ts.Add("FuncName", { (int)funcName.length() + 1 },
			sizeof(char), TENSOR_DATA_TYPES::BYTE);
		auto tsFuncName = ts["FuncName"];
		memcpy(tsFuncName.D<char>(), funcName.c_str(), funcName.length() + 1);

		df = ts;

		mRunningLock.Lock();
		mRunnings.emplace(std::make_pair(pInst->id,pInst));
		mRunningLock.Unlock();
		//TODO(Shawn@10/4/2021): consider lock for this worker
		pWorker->mCurrentTaskInstance = pInst;
		pWorker->mTask = pTaskInfo;

		bHaveTask = true;
	}
	mInstanceLock.Unlock();
	if (pWorker)
	{
		pWorker->DecRef();
	}
	return bHaveTask;
}

bool DC::TaskManager::SetTaskStatusForWorker(DataFrame& df)
{
	bool bOK = false;
	TensorFrame tsFrm(df);
	Tensor tsInstID = tsFrm["TaskInstanceID"];
	Tensor tsRetValues = tsFrm["ReturnValues"];
	UID instId =UIDFromString(tsInstID.D<char>());
	if (instId!=UID() && !tsRetValues.IsEmpty())
	{
		bOK = DC::StoreManager::I().Set(
			instId, tsRetValues.D<char>(), tsRetValues.Size());
	}

	return bOK;
}
void DC::TaskManager::RegisterTask(X::XRuntime* rt,
	X::XObj* pContext, 
	X::ARGS& params,
	X::KWARGS& kwParams,
	X::Value& funcBytes,
	X::Value& retValue)
{
	std::cout << "DC::TaskManager::RegisterTask" << std::endl;
	X::Value funcStream;
	if (!funcBytes.IsObject() || funcBytes.GetObj()->GetType() != X::ObjType::Binary)
	{
		X::g_pXHost->ToBytes(funcBytes, funcStream);
	}
	else
	{
		funcStream = retValue;
	}
	auto* pXPack = APISET().GetPack();
	X::XPackageValue<TASK> valTasker;
	(*valTasker).SetOriginalObject(funcStream);
	retValue = valTasker;
}

void DC::TASK::Run(X::XRuntime* rt, X::XObj* pContext,
	X::ARGS& params, X::KWARGS& kwParams, X::Value& retValue)
{
	std::cout << "DC::Tasker::Run" << std::endl;
	//Pack as DataFrame
	DataFrame frm;
	frm.head->type = (FrameType)'task';
	frm.head->startTime = getCurMilliTimeStamp();
	X::XBin* pBin = dynamic_cast<X::XBin*>(m_originalObject.GetObj());
	auto size = frm.head->dataSize = pBin->Size();
	frm.data = new char[size];
	memcpy(frm.data, pBin->Data(), size);
	CantorHostImpl::I().PushFrame(frm);
	X::XPackageValue<TaskInstance> valInstance;
	(*valInstance).mTask = this;
	retValue = valInstance;
}
void DC::TaskInstance::Get(X::XRuntime* rt, X::XObj* pContext,
	X::ARGS& params, X::KWARGS& kwParams, X::Value& retValue)
{
	std::cout << "DC::TaskInstance::Get" << std::endl;
}