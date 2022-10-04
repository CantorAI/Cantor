#ifndef _XHOST_H_
#define _XHOST_H_
#include <string>
#include <vector>
#include "frame.h"

class CantorHost
{
public:
    virtual void Print(std::string strInfo) = 0;
    virtual void Print(std::wstring strInfo) = 0;
	virtual DataViewID CreateDataView(std::string& filter) =0;
	virtual bool RemoveDataView(DataViewID id) =0;
	virtual bool PushFrame(const DataFrame& frm) =0;
	virtual bool PopFrame(DataViewID dvId, TimeStamp ts, int timeout_ms, DataFrame& frm)=0;
};

#endif