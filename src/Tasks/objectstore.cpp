#include "objectstore.h"
#include "CantorHost.h"
#include "CantorWait.h"
#include "service_def.h"
#include "TaskManager.h"
#include "utility.h"

DC::StoreManager::StoreManager()
{
}

DC::StoreManager::~StoreManager()
{
}

bool DC::StoreManager::SetValue(X::Value& v, X::Value& retObjRef)
{
	auto strId = DC::TaskManager::I().GenGuid();
	UID id = UIDFromString(strId);
	bool bOK = Create(id);
	if (!bOK)
	{
		return false;
	}
	X::Value valBytes;
	if (!v.IsObject() || v.GetObj()->GetType() != X::ObjType::Binary)
	{
		X::g_pXHost->ToBytes(v, valBytes);
	}
	else
	{
		valBytes = v;
	}
	X::XBin* pBin = dynamic_cast<X::XBin*>(valBytes.GetObj());
	bOK =  Set(id, pBin->Data(), pBin->Size());
	if (bOK)
	{
		UID nodeId= CantorHostImpl::I().GetNodeId();
		constexpr int buf_len = 100;
		char objRef[buf_len] = {};
		SPRINTF(objRef, buf_len, "!%016llx%016llx.%016llx%016llx",
			nodeId.h, nodeId.l,id.h, id.l);
		retObjRef = objRef;
	}
	return bOK;
}
X::Value DC::StoreManager::GetValue(std::string& strObjRef, int timeout)
{
	UID uid;
	UID nodeId;
	int retVal = sscanf(strObjRef.c_str(), "!%016llx%016llx.%016llx%016llx",
		&nodeId.h, &nodeId.l,&uid.h, &uid.l);
	if (retVal != 4)
	{
		return X::Value();
	}
	unsigned long size = 0;
	bool bOK = QuerySizeAndStatus(uid, size, timeout);
	if (bOK)
	{
		char* pBuf = new char[size];
		Get(uid, pBuf, size);
		X::Value valBuf(X::g_pXHost->CreateBin(pBuf, size));
		X::Value retVal;
		X::g_pXHost->FromBytes(valBuf, retVal);
		return retVal;
	}
	else
	{
		return X::Value();
	}
}
bool DC::StoreManager::Create(UID& id)
{
	//create an empty item inside mObjs
	ObjectRecord* rec = new ObjectRecord();
	mObjLock.Lock();
	mObjs.emplace(std::make_pair(id, rec));
	mObjLock.Unlock();

	return true;
}

bool DC::StoreManager::Set(UID& id,
	const char* data, unsigned long size)
{
	bool bOK = false;
	ObjectRecord* rec = nullptr;

	mObjLock.Lock();
	auto it = mObjs.find(id);
	if (it != mObjs.end())
	{
		rec = it->second;
		bOK = true;
	}
	mObjLock.Unlock();
	if (bOK)
	{
		bOK = rec->Set(mHost,data, size);
	}

	return bOK;
}

bool DC::StoreManager::QuerySizeAndStatus(UID& id,
	unsigned long& size, int timeout)
{
	bool bOK = false;
	ObjectRecord* rec = nullptr;

	mObjLock.Lock();
	auto it = mObjs.find(id);
	if (it != mObjs.end())
	{
		rec = it->second;
		bOK = true;
	}
	mObjLock.Unlock();
	if (bOK)
	{//we directly use fields inside ObjectRecord
		//because we don't want ObjectRecord keep mHost as its member
		//to save memory
		CantorWait* waitHandle = nullptr;
		rec->Lock();
		if (rec->mStatus == (unsigned short)Status::Ready)
		{
			size = rec->mSize;
			bOK = true;
		}
		else
		{
			bOK = false;
			waitHandle = new CantorWait();
			rec->mWaits.push_back(waitHandle);
		}
		rec->Unlock();
		if (!bOK && waitHandle!= nullptr)
		{
			if (waitHandle->Wait(timeout))
			{
				rec->Lock();
				size = rec->mSize;
				bOK = true;
				rec->Unlock();
			}
			rec->Lock();
			for (auto itw = rec->mWaits.begin(); 
				itw != rec->mWaits.end(); itw++)
			{
				rec->mWaits.erase(itw);
				break;
			}
			rec->Unlock();
			delete waitHandle;
		}
	}
	return bOK;
}

bool DC::StoreManager::Get(UID& id,
	const char* buffer, unsigned long& bufSize)
{
	bool bOK = false;
	ObjectRecord* rec = nullptr;

	mObjLock.Lock();
	auto it = mObjs.find(id);
	if (it != mObjs.end())
	{
		rec = it->second;
		bOK = true;
	}
	mObjLock.Unlock();
	if (rec)
	{
		bOK = rec->Copy(buffer, bufSize);
	}
	return bOK;
}

int DC::StoreManager::AddObjRef(UID& id, int cntAdd)
{
	int refCnt = -1;//error
	ObjectRecord* rec = nullptr;
	mObjLock.Lock();
	auto it = mObjs.find(id);
	if (it != mObjs.end())
	{
		rec = it->second;
		if (cntAdd > 0)
		{
			refCnt = rec->IncRef();
		}
		else
		{
			refCnt = rec->DecRef();
		}
		if (refCnt == 0)
		{
			auto it = mObjs.find(id);
			if (it != mObjs.end())
			{
				mObjs.erase(it);
			}
		}
	}
	mObjLock.Unlock();
	if (refCnt == 0 && rec!=nullptr)
	{
		delete rec;
	}
	return refCnt;
}

DC::ObjectRecord::ObjectRecord()
	:mDataPtr(0)
{
}


DC::ObjectRecord::~ObjectRecord()
{
}

bool DC::ObjectRecord::Set(CantorHost* pHost, 
	const char* data,unsigned long size)
{
	Lock();
	mSize = size;
	if (size <= IN_BLOCK_SIZE)
	{
		if (mData == nullptr)
		{
			mData = new char[size];
		}
		memcpy(mData, data, size);
	}
	else
	{
		////TODO(shawn@10/8/2021): put into memory or store
	}
	mStatus = (unsigned short)Status::Ready;
	for (auto w : mWaits)
	{
		delete w;
	}
	Unlock();
	return true;
}

bool DC::ObjectRecord::Copy(const char* buffer,
	unsigned long& bufSize)
{
	bool bOK = false;
	Lock();
	unsigned long cpSize = (mSize > bufSize) ? bufSize : mSize;
	if (mSize <= IN_BLOCK_SIZE)
	{
		memcpy((void*)buffer, mData, cpSize);
		bOK = true;
	}
	else
	{
		//From other memory
	}
	Unlock();

	bufSize = cpSize;

	return bOK;
}

int DC::ObjectRecord::DecRef()
{
	int refCnt = 0;
	Lock();
	refCnt = --mRefCount;
	Unlock();
	return refCnt;
}

int DC::ObjectRecord::IncRef()
{
	int refCnt = 0;
	Lock();
	refCnt = ++mRefCount;
	Unlock();
	return refCnt;
}
