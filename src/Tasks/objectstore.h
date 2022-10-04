#pragma once
#include "singleton.h"
#include <unordered_map>
#include <vector>
#include <string>
#include "Locker.h"
#include "cantor_host.h"
#include "gxydef.h"
#include "NodeManager.h"
#include "xlang.h"

class CantorWait;
class CantorHost;
namespace DC {
#define IN_BLOCK_SIZE (1024*1024)
	enum class Status
	{
		Empty = 0,
		Ready
	};
	enum class Head
	{
		Data_From_Data_Field,
		Data_From_Memory,
		Data_From_Storage
	};
	class ObjectRecord
	{
		friend class StoreManager;
	public:
		ObjectRecord();

		~ObjectRecord();

		bool Set(CantorHost* pHost, const char* data, unsigned long size);
		bool Copy(const char* buffer, unsigned long& bufSize);
		inline void Lock()
		{
			locker.Lock();
		}
		inline void Unlock()
		{
			locker.Unlock();
		}
		int DecRef();
		int IncRef();

	private:
		//if record's size <= IN_BLOCK_SIZE
		//just save to mData,
		//else mDataPtr points to other memory location
		unsigned short mHead = 0;
		unsigned short mStatus = 0;//from enum Status
		unsigned long mSize = 0;

		union
		{
			unsigned long long mDataPtr;
			char* mData;
		};

		Locker locker;
		unsigned int mRefCount = 1; //Data's RefCount
		std::vector<CantorWait*> mWaits;
	};
	class StoreManager :
		public Singleton<StoreManager>
	{
	public:
		StoreManager();
		~StoreManager();
		void SetHost(CantorHost* p)
		{
			mHost = p;
		}
		bool SetValue(X::Value& v, X::Value& retObjRef);
		X::Value GetValue(std::string& strObjRef,int timeout);
		bool Create(UID& id);
		bool Set(UID& id, const char* data, unsigned long size);
		//after data record created,immutable
		bool QuerySizeAndStatus(UID& id, unsigned long& size, int timeout);
		bool Get(UID& id, const char* buffer, unsigned long& bufSize);
		int AddObjRef(UID& id, int cntAdd);
	private:
		CantorHost* mHost = nullptr;
		Locker mObjLock;
		std::unordered_map<UID, ObjectRecord*, UIDHashFunction> mObjs;
	};
}
