#include "workermanager.h"
#include "CantorHost.h"
#include "utility.h"

DC::WorkerManager::WorkerManager()
{
}

DC::WorkerManager::~WorkerManager()
{
}

DC::Worker* DC::WorkerManager::QueryOrAddWorker(
	unsigned long workerProcessId)
{
	Worker* pWorker = nullptr;
	mWorkLock.Lock();
	auto it = mWorkers.find(workerProcessId);
	if (it == mWorkers.end())
	{
		pWorker = new Worker();
		pWorker->processId = workerProcessId;
		mWorkers.emplace(std::make_pair(workerProcessId, pWorker));
	}
	else
	{
		pWorker = it->second;
	}
	mWorkLock.Unlock();
	if (pWorker)
	{
		pWorker->AddRef();
	}
	return pWorker;
}

bool DC::WorkerManager::SetWorkerStatus(unsigned long workerProcessId, WorkerStatus s)
{
	bool bHave = false;
	mWorkLock.Lock();
	auto it = mWorkers.find(workerProcessId);
	if (it != mWorkers.end())
	{
		Worker* pWorker = it->second;
		if (pWorker)
		{
			pWorker->SetStatus(s);
			bHave = true;
		}
	}
	mWorkLock.Unlock();
	return bHave;
}

DC::Worker* DC::WorkerManager::FindOrCreateIdelWorker(bool bWaitForReady)
{
	Worker* pWorker = nullptr;
	mWorkLock.Lock();
	for (auto& it : mWorkers)
	{
		if (it.second->GetStatus() == WorkerStatus::Ready)
		{
			pWorker = it.second;
			break;
		}
	}
	mWorkLock.Unlock();

	if (pWorker == nullptr)
	{
		std::string initPath = mHost->GetHostPath();
		unsigned long workerProcessId = RunPythonWorker(initPath);
		pWorker = new Worker();
		pWorker->processId = workerProcessId;
		mWorkLock.Lock();
		mWorkers.emplace(std::make_pair(workerProcessId, pWorker));
		mWorkLock.Unlock();
		if (bWaitForReady)
		{
			pWorker->AddRef();
			while (pWorker->GetStatus() != WorkerStatus::Ready 
				&& pWorker->GetStatus() != WorkerStatus::Stopped)
			{
				pWorker->WaitForStatusChange();
			}
			pWorker->DecRef();
		}
	}
	if (pWorker)
	{
		if (pWorker->GetStatus() == WorkerStatus::Ready)
		{
			pWorker->AddRef();
		}
		else if (pWorker->GetStatus() == WorkerStatus::Stopped)
		{
			pWorker = nullptr;// if created here, will be released by worker close event from mWorkers
		}
	}
	return pWorker;
}

void DC::WorkerManager::RemoveWorker(unsigned long workerProcessId)
{
	mWorkLock.Lock();
	auto it = mWorkers.find(workerProcessId);
	if (it != mWorkers.end())
	{
		Worker* pWorker = it->second;
		if (pWorker)
		{
			pWorker->SetStatus(WorkerStatus::Stopped);
			mWorkers.erase(it);
			pWorker->DecRef();
		}
	}
	mWorkLock.Unlock();
}

unsigned long  DC::WorkerManager::RunPythonWorker(std::string initPath)
{
	std::string cmd = "python -c \""\
		"import cantor;"
		"cantor.EnterMainLoop();\"";
	unsigned long processId = 0;
	RunProcess(cmd, initPath, processId);
	return processId;
}

WorkerStatus DC::Worker::GetStatus()
{
	WorkerStatus s;
	mLock.Lock();
	s = mStatus;
	mLock.Unlock();

	return s;
}

void DC::Worker::SetStatus(WorkerStatus s)
{
	mLock.Lock();
	mStatus = s;
	for (auto it : m_waits)
	{
		it->Release();
	}
	mLock.Unlock();
}

bool DC::Worker::WaitForStatusChange(int timeout_ms)
{
	CantorWait* pWait = new CantorWait();
	mLock.Lock();
	m_waits.push_back(pWait);
	mLock.Unlock();
	bool bOK = pWait->Wait(timeout_ms);
	mLock.Lock();
	for (auto it = m_waits.begin(); it != m_waits.end();)
	{
		if (*it == pWait)
		{
			m_waits.erase(it);
			break;
		}
	}
	mLock.Unlock();
	delete pWait;
	return bOK;
}
