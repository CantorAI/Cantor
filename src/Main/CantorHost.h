#pragma once

#include "cantor_host.h"
#include "Locker.h"
#include "singleton.h"
#include "frame.h"
#include <unordered_map>
#include "xhost.h"
#include "xpackage.h"
#include "xlang.h"

class DataView;
class CantorHostImpl :
	public CantorHost,
	public Singleton<CantorHostImpl>
{
public:
	CantorHostImpl();
	~CantorHostImpl();
	BEGIN_PACKAGE(CantorHostImpl)
		ADD_FUNC(1,"PushFrame", &CantorHostImpl::PushFrameAPI)
		ADD_FUNC(1,"CreateDataView", &CantorHostImpl::CreateDataViewAPI)
		ADD_FUNC(1,"RemoveDataView", &CantorHostImpl::RemoveDataView)
		ADD_FUNC(3,"PopFrame", &CantorHostImpl::PopFrameAPI)
		ADD_FUNC(0,"generate_uid", &CantorHostImpl::Generate_UID)
	END_PACKAGE

	bool Start();
	void Shutdown();
	//For API
	inline bool PushFrameAPI(X::Value& valFrame)
	{
		return PushFrame(X::XPackageValue<DataFrame>(valFrame));
	}
	X::Value PopFrameAPI(DataViewID dvId, TimeStamp ts, int timeout_ms);
	inline DataViewID CreateDataViewAPI(std::string filter)
	{
		return CreateDataView(filter);
	}
	std::string Generate_UID();
	virtual void Print(std::string strInfo) override;
	virtual void Print(std::wstring strInfo) override;

	virtual DataViewID CreateDataView(std::string& filter) override;
	virtual bool RemoveDataView(DataViewID id) override;
	virtual bool PushFrame(const DataFrame& frm) override;
	virtual bool PopFrame(DataViewID dvId,TimeStamp ts,int timeout_ms,DataFrame& frm) override;

	std::string GetNodeName();
	UID GetNodeId();
	std::string& GetHostPath()
	{
		return mHostPath;
	}
	void SetHostPath(std::string& path)
	{
		mHostPath = path;
	}
	void ReleaseDataViewWaits(DataViewID id);
private:
	std::string mHostPath;
	Locker mDataViewLock;
	std::unordered_map<DataViewID,DataView*> mDVMap;
};