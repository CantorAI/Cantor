#include "singleton.h"
#include "Locker.h"
#include "TaskManager.h"
#include "service_def.h"
#include "CantorWait.h"

class CantorHostImpl;

namespace DC //Distributed Computing
{
	class Worker
	{
	public:
		Worker():
			mRef(1)
		{
		}
		unsigned long processId = 0;//if run as thread, this == 0
		unsigned long threadId = 0;//if run as porcess, this ==0
		TASK* mTask = nullptr;//the last running instance's task
		TaskInstance* mCurrentTaskInstance = nullptr;
		WorkerStatus GetStatus();
		void SetStatus(WorkerStatus s);
		bool WaitForStatusChange(int timeout_ms =-1);
		int AddRef()
		{
			int ref;
			mLock.Lock();
			ref = ++mRef;
			mLock.Unlock();
			return ref;
		}
		void DecRef()
		{
			int ref=0;
			mLock.Lock();
			ref = --mRef;
			mLock.Unlock();
			if (ref == 0)
			{
				delete this;
			}
		}
	private:
		Locker mLock;
		std::vector<CantorWait*> m_waits;
		WorkerStatus mStatus = WorkerStatus::None;
		int mRef = 0;
	};

	class WorkerManager :
		public Singleton<WorkerManager>
	{
	public:
		WorkerManager();
		~WorkerManager();
		void SetHost(CantorHostImpl* p)
		{
			mHost = p;
		}
		Worker* QueryOrAddWorker(unsigned long workerProcessId);
		Worker* FindOrCreateIdelWorker(bool bWaitForReady);
		void RemoveWorker(unsigned long workerProcessId);
	private:
		bool SetWorkerStatus(unsigned long workerProcessId, WorkerStatus s);
		unsigned long RunPythonWorker(std::string initPath);
		Locker mWorkLock;
		std::map<unsigned long, Worker*> mWorkers;

		CantorHostImpl* mHost = nullptr;
	};
}