#include "DataFramePack.h"
#include "port.h"

DataFramePack::DataFramePack(unsigned int idx)
	:m_DataIndex(idx)
{
}

DataFramePack::~DataFramePack()
{
}

bool DataFramePack::AppendData(char* data, int size)
{
	//for packet
	m_CurPacketFillSize += size;
	//for DataFrame's head
	if (mHeadFillSize < (int)sizeof(DataFrameHead))
	{
		int needSize = sizeof(DataFrameHead) - mHeadFillSize;
		if (needSize > size)
		{
			needSize = size;
		}
		memcpy(((char*)m_frame.head) + mHeadFillSize,
			data, needSize);
		mHeadFillSize += needSize;
		size -= needSize;
		data += needSize;
	}
	if (mHeadFillSize < (int)sizeof(DataFrameHead))
	{
		//head is not full,need more data coming
		return false;
	}
	//Fill Data
	if (m_frame.data == nullptr)
	{
		m_frame.data = new char[m_frame.head->dataSize];
	}
	memcpy(m_frame.data + mDataFillSize, data, size);
	mDataFillSize += size;
	//check DataFrame is full or not
	return (mDataFillSize == m_frame.head->dataSize);
}
