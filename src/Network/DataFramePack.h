#pragma once

#include "frame.h"
class DataFramePack
{
public:
	DataFramePack(unsigned int idx);
	~DataFramePack();
	inline unsigned int GetCurPacketFillSize()
	{
		return m_CurPacketFillSize;
	}
	inline void ResetForNextPacket()
	{
		m_CurPacketFillSize = 0;
	}
	bool AppendData(char* data, int size);
	DataFrame& GetFrame()
	{
		return m_frame;
	}
private:
	unsigned int m_DataIndex = 0;
	unsigned int m_CurPacketFillSize = 0;

	DataFrame m_frame;
	int mHeadFillSize=0;//for DataFrameHead
	unsigned long long mDataFillSize=0;
};