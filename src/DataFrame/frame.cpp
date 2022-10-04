#include "frame.h"
#include <string.h>


DataFrame::DataFrame() :
	head(new DataFrameHead()),
	locker(new Locker()),
	cached(new bool),
	ref(new unsigned int)
{
	*cached = false;
	*ref = 1;
}

DataFrame::DataFrame(const DataFrame& f)
{
	Copy(f);
}


DataFrame::~DataFrame()
{
	DecRef();
}

DataFrame& DataFrame::operator = (const DataFrame& f)
{
	DecRef(); //Reduce refcount,may destroy old DataFrame data
	Copy(f);
	return *this;
}


void DataFrame::DecRef()
{
	Lock();
	--(*ref);
	if ((*ref) == 0 && !(*cached))
	{
		delete ref;
		ref = nullptr;
		delete cached;
		cached = nullptr;
		Unlock();
		delete locker;
		locker = nullptr;
		ReleaseMemory();
	}
	else
	{
		Unlock();
	}
}

void DataFrame::ReleaseMemory()
{
	if (data)
	{
		delete[] data; // must be array
		data = nullptr;
	}
	if (head)
	{
		delete head;
		head = nullptr;
	}
}

void DataFrame::Copy(const DataFrame& f)
{
	f.locker->Lock();
	locker = f.locker;
	cached = f.cached;
	ref = f.ref;
	++(*ref);
	f.locker->Unlock();

	context = f.context;
	session = f.session;
	head = f.head;
	data = f.data;
}

void DataFrame::Clone(const DataFrame& f)
{
	context = f.context;
	session = f.session;
	if (head != nullptr)
	{
		delete head;
	}
	if (data != nullptr)
	{
		delete[] data;
	}
	head = new DataFrameHead();
	*head = *f.head;
	data = new char[head->dataSize];
	memcpy(data, f.data, head->dataSize);
}
void DataFrame::Zero()
{
	Lock();
	memset(data, 0, head->dataSize);
	Unlock();
}
