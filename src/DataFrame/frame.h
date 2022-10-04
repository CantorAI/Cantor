#ifndef __frame_h__
#define __frame_h__

#include "Locker.h"
#include <string>

const int FMT_NUM = 8;
const int METADATA_NUM = 4;
class UID
{
public:
	unsigned long long h;
	unsigned long long l;

	UID()
	{
		h = l = 0;
	}
	bool IsNull()
	{
		return h ==0 && l == 0;
	}
	UID(const UID& f)
	{
		h = f.h;
		l = f.l;
	}

	UID& operator=(const UID& f) 
	{
		h = f.h;
		l = f.l;
		return *this;
	}

	bool operator<(const UID& r) const
	{
		return (h < r.h || (h == r.h && l < r.l));
	}
	bool operator==(const UID& r) const
	{
		return (h == r.h && l == r.l);
	}
	bool operator!=(const UID& r) const
	{
		return (h != r.h || l != r.l);
	}
};

typedef unsigned long long FrameType;
typedef unsigned long long TimeStamp;
typedef unsigned long long FormatType;
typedef UID SourceID;
typedef UID Address;
typedef void* DataViewID;

#define DF_FMT_NUM 8
#define DF_FMT_NUM 8

#define ANY_FRAME_TYPE 0
#define NO_FRAME_TYPE -1 // as error

struct DataFrameHead
{
	unsigned int tag = 0x6466726d; //'dfrm';
	unsigned int version = 0;

	FrameType  type = 0;
	TimeStamp startTime = 0;

	SourceID sourceId;
	Address srcAddr;
	Address dstAddr;

	FormatType format[DF_FMT_NUM] = { 0 };
	unsigned long long metadata[METADATA_NUM] = { 0 };

	unsigned int refId = 0;
	unsigned int refIndex = 0;

	unsigned long long dataSize = 0;
	unsigned long long dataItemNum = 0;//bytesdata, if >=1, Key(string):Val 
};

class DataFrame
{
public:
	DataFrame();
	DataFrame(const DataFrame& f);

	virtual ~DataFrame();

	DataFrame& operator=(const DataFrame& f);
	void Clear()
	{
		DecRef();
	}
	void DecRef();
	inline bool IsEmpty()
	{
		return (data == nullptr);
	}
	inline bool IsCached() const
	{
		Lock();
		bool r = *cached;
		Unlock();
		return r;
	}
	void Zero();
	inline void Lock() const
	{
		locker->Lock();
	}
	inline void Unlock() const
	{
		locker->Unlock();
	}
	inline void SetCached(bool v) const
	{
		Lock();
		*cached = v;
		Unlock();
	}

public:
	//head and data pass over network and storage
	DataFrameHead* head = nullptr;
	char* data = nullptr;

	//only valid in current process
	void* context = nullptr;
	void* session = nullptr;
	void Clone(const DataFrame& f);
protected:
	void ReleaseMemory();
	void Copy(const DataFrame& f);

	Locker* locker = nullptr;
	bool* cached;
	unsigned int* ref; //Data's RefCount
};

#endif//_frame_h__