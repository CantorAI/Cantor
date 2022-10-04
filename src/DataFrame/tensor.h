#ifndef __tensor_h__
#define __tensor_h__

#include "frame.h"
#include <string>
#include <vector>
#include <map>
#include <stdarg.h>  
#include <memory.h>
#include <string.h>

struct header
{
	int itemDataType;
	int itemDataSize;
	int nd;
};
class Tensor
{
public:
	Tensor();
	Tensor(const Tensor& td);
	Tensor(char* dataptr);
	bool IsEmpty()
	{
		return (head == nullptr);
	}
	Tensor& operator=(const Tensor& f);

	template<typename T> T* D() { return (T*)data; }
	template<typename T>
	T& Item(int idx0, ...)
	{
		//For array A of size  n¡Ám¡Áp , the address of A[i][j][k] is  A+i¡Ám¡Áp+j¡Áp+k.
		int addr = idx0 * dimProd[0];
		va_list vl;
		va_start(vl, idx0);
		for (int i = 1; i < head->nd; i++)
		{
			addr += va_arg(vl, int) * dimProd[i];
		}
		va_end(vl);
		return ((T*)data)[addr];
	}
	unsigned long long Size()
	{
		unsigned long long len = head->itemDataSize;
		for (int i = 0; i < head->nd; i++)
		{
			len *= pdims[i];
		}
		return len;
	}
	header& Head() { return *head; }
	unsigned long long* Dims() { return pdims; }
protected:
	void Copy(const Tensor& f);
	std::vector<int> dimProd;
	header* head;
	unsigned long long* pdims;
	char* data;//reference to real data's memmory
};

struct TensorPackInfo
{
	std::string key;
	int itemDataType;
	int itemDataSize;
	int nd;
	unsigned long long* pdims;
	char* data;
};

//copy from C:\Python38\Lib\site-packages\numpy\core\include\numpy\ndarraytypes.h
enum class TENSOR_DATA_TYPES
{
	BOOL = 0,
	BYTE, UBYTE,
	SHORT, USHORT,
	INT, UINT,
	LONG, ULONG,
	LONGLONG, ULONGLONG,
	FLOAT, DOUBLE, LONGDOUBLE,
	CFLOAT, CDOUBLE, CLONGDOUBLE,
	OBJECT = 17,
	STRING, UNICODE,
	VOIDTYPE,
	DATAFRAME,//for DataFrame
};
class TensorVariant
{
public:
	TensorVariant()
	{

	}
	~TensorVariant()
	{
		if (type == TENSOR_DATA_TYPES::STRING)
		{
			delete (char*)data;
		}
	}
	TensorVariant(int v)
	{
		data = v;
		type = TENSOR_DATA_TYPES::INT;
	}
	operator int() const { return (int)data; }

	TensorVariant(long v)
	{
		data = v;
		type = TENSOR_DATA_TYPES::LONG;
	}
	operator long() const { return (long)data; }

	TensorVariant(long long v)
	{
		data = v;
		type = TENSOR_DATA_TYPES::LONGLONG;
	}
	operator long long() const { return (long long)data; }

	TensorVariant(float v)
	{
		data = v;
		type = TENSOR_DATA_TYPES::FLOAT;
	}
	operator float() const { return (float)data; }

	TensorVariant(double v)
	{
		data = v;
		type = TENSOR_DATA_TYPES::DOUBLE;
	}
	operator double() const { return (double)data; }

	TensorVariant(std::string v)
	{
		char* sz = new char[v.size()];
		memcpy(sz, v.c_str(), v.size());
		data = (unsigned long long)sz;
		type = TENSOR_DATA_TYPES::STRING;
	}
	TensorVariant(const char* v)
	{
		auto size = strlen(v) + 1;
		char* sz = new char[size];
		memcpy(sz, v, size);
		data = (unsigned long long)sz;
		type = TENSOR_DATA_TYPES::STRING;
	}
	operator char* () const { return (char*)data; }

protected:
	TENSOR_DATA_TYPES type = TENSOR_DATA_TYPES::BOOL;
	unsigned long long data = 0;//8 bytes to hold pointer or value
};
class TensorFrame :
	public DataFrame
{
public:
	TensorFrame();
	TensorFrame(const DataFrame& f);
	TensorFrame(const TensorFrame& t);

	virtual ~TensorFrame();

	TensorFrame& operator=(const DataFrame& f);
	TensorFrame& operator=(const TensorFrame& f);

	Tensor& operator[](const char* key);
	std::map<std::string, Tensor>& KV()
	{
		return mKV;
	}
	void Init(std::vector<TensorPackInfo>& packs);

	bool Add(const char* skey, std::vector<int> dims,
		int itemDataSize, TENSOR_DATA_TYPES itemDataType, char* da = nullptr);
#if 0
	template<typename T>
	bool Add(const char* skey, std::vector<int> dims, char* da = nullptr)
	{
		return Add(skey, dims, sizeof(T), da);
	}
#endif
	void Clone(const TensorFrame& f);

protected:
	void Copy(const DataFrame& f);
	void Copy(const TensorFrame& f);
protected:
	void ParseData();
	std::map<std::string, Tensor> mKV;
	Tensor mEmptyTensor;
};
#endif//__tensor_h__
