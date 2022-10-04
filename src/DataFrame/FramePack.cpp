#include "FramePack.h"
#include <memory.h>

PackDataFrame::PackDataFrame(DataFrame& frm)
{
	m_frm = &frm;//keep in same stack,so it is safe
}

bool PackDataFrame::Finish()
{
	char* pData = new char[m_frm->head->dataSize];
	m_frm->data = pData;
	for (auto& it : m_mems)
	{
		switch (it.tp)
		{
		case MemType::Pointer:
			memcpy(pData, it.m.p, it.size);
			pData += it.size;
			break;
		case MemType::Int:
			*(int*)pData = it.m.i;
			pData += sizeof(int);
			break;
		case MemType::Char:
			*(char*)pData = it.m.c;
			pData += sizeof(char);
			break;
		case MemType::Int64:
			*(long long*)pData = it.m.ll;
			pData += sizeof(long long);
			break;
		case MemType::Double:
			*(double*)pData = it.m.d;
			pData += sizeof(double);
			break;
		case MemType::Stream:
			*(unsigned long long *)pData = it.size;
			pData += sizeof(unsigned long long);
			it.m.pStream->FullCopyTo(pData, it.size);
			X::g_pXHost->ReleaseStream(it.m.pStream);
			pData += it.size;
			break;
		default:
			break;
		}
	}
	return true;
}
