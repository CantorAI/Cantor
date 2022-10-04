#include "WorkerMode.h"
#include "gxydef.h"
#include "xhost.h"
#include "xlang.h"
#include "service_def.h"

void WorkerMode::Loop()
{
	X::XRuntime* rt = X::g_pXHost->CreateRuntime();
	rt->CreateEmptyModule();
	X::Value v0;
	X::g_pXHost->Import(rt, "cantor", nullptr, "lrpc:1000",v0);
	X::XObj cantor(v0);
	X::XObj host(cantor["Host"]());
	std::string id = (std::string)host["generate_uid"]();

	std::string filter = "Type == 100";
	constexpr int buf_len = 100;
	char szFilter[buf_len]={};
	SPRINTF(szFilter, buf_len,"type == %d",'task');
	std::string strFilter(szFilter);
	auto dvId = host["CreateDataView"](strFilter);
	auto popFrame = host["PopFrame"];
	while (true)
	{
		X::XObj frm = popFrame(dvId,0,-1);
		auto data = frm["GetData"]();
		X::Value funcToRun;
		bool bOK = X::g_pXHost->FromBytes(data,funcToRun);
		MS_SLEEP(100);
	}
	host["RemoveDataView"](dvId);
	//TODO: check here can use delete or not
	delete rt;
}
