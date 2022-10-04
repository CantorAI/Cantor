#include "singleton.h"
#include "Locker.h"
#include "gthread.h"

class CantorHost;

namespace DC //Distributed Computing
{
	class RemotingMethodProcess :
		public GThread,
		public Singleton<RemotingMethodProcess>
	{
	public:
		RemotingMethodProcess();
		~RemotingMethodProcess();
		void SetHost(CantorHost* p)
		{
			mHost = p;
		}
	private:
		CantorHost* mHost = nullptr;
		
		bool mRun = true;
		// Inherited via GThread
		virtual void run() override;
	};
}