#pragma once
#include "frame.h"
#include "xlang.h"
#include "xhost.h"
#include "xlstream.h"
#include <vector>


class PackDataFrame
{
	enum class MemType
	{
		Pointer,
		Char,
		Int,
		Int64,
		Double,
		Stream
	};
	struct MemInfo
	{
		MemType tp= MemType::Pointer;
		int r = 0;//for 8-bytes alignment
		union M
		{
			char c;
			int i;
			long long ll;
			double d;
			void* p;
			X::XLStream* pStream;
		}m;
		size_t size=0;
	};
public:
	PackDataFrame():
		m_frm(nullptr),
		m_pCurDataPtr(nullptr)
	{
	}
	PackDataFrame(DataFrame& frm);

	PackDataFrame& operator<<(const char v)
	{
		m_frm->head->dataSize += sizeof(v);
		MemInfo item;
		item.tp = MemType::Char;
		item.m.c = v;
		m_mems.push_back(item);
		return *this;
	}
	PackDataFrame& operator<<(const int v)
	{
		m_frm->head->dataSize += sizeof(v);
		MemInfo item;
		item.tp = MemType::Int;
		item.m.i = v;
		m_mems.push_back(item);
		return *this;
	}
	PackDataFrame& operator<<(const double v)
	{
		m_frm->head->dataSize += sizeof(v);
		MemInfo item;
		item.tp = MemType::Double;
		item.m.d = v;
		m_mems.push_back(item);
		return *this;
	}
	template<typename T>
	PackDataFrame& operator<<(const T& v)
	{
		m_frm->head->dataSize += sizeof(v);
		MemInfo item;
		item.tp = MemType::Pointer;
		item.m.p = (void*)&v;
		item.size = sizeof(v);
		m_mems.push_back(item);
		return *this;
	}
	PackDataFrame& operator<<(X::Value& v)
	{
		X::XLStream* pStream = X::g_pXHost->CreateStream();
		v.ToBytes(pStream);
		auto size = pStream->Size();
		m_frm->head->dataSize += sizeof(size) + size;
		MemInfo item;
		item.tp = MemType::Stream;
		item.m.pStream = pStream;
		item.size = sizeof(size)+size;//first 8 bytes for size of Value
		m_mems.push_back(item);
		return *this;
	}
	PackDataFrame& operator<<(std::string& v)
	{
		m_frm->head->dataSize += v.size()+1;
		MemInfo item;
		item.tp = MemType::Pointer;
		item.m.p = (void*)v.c_str();
		item.size = v.size() + 1;
		m_mems.push_back(item);
		return *this;
	}

	template<typename T>
	PackDataFrame& operator>>(T& v)
	{
		if (m_pCurDataPtr == nullptr)
		{
			m_pCurDataPtr = m_frm->data;
		}
		v = *(T*)m_pCurDataPtr;
		m_pCurDataPtr += sizeof(T);
		return *this;
	}
	PackDataFrame& operator>>(X::Value& v)
	{
		if (m_pCurDataPtr == nullptr)
		{
			m_pCurDataPtr = m_frm->data;
		}
		unsigned long long size = *(unsigned long long*)m_pCurDataPtr;
		m_pCurDataPtr += sizeof(unsigned long long);
		X::XLStream* pStream = X::g_pXHost->CreateStream(m_pCurDataPtr, size);
		v.FromBytes(pStream);
		X::g_pXHost->ReleaseStream(pStream);
		m_pCurDataPtr += size;
		return *this;
	}
	PackDataFrame& operator>>(std::string& v)
	{
		if (m_pCurDataPtr == nullptr)
		{
			m_pCurDataPtr = m_frm->data;
		}
		v = m_pCurDataPtr;
		m_pCurDataPtr += v.size() + 1;
		return *this;
	}

	bool Finish();
protected:
	DataFrame* m_frm=nullptr;
	std::vector<MemInfo> m_mems;
	char* m_pCurDataPtr = nullptr;
};
