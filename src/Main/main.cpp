#include "main.h"
#include <string>
#include <signal.h>
#include "log.h"
#include "gxydef.h"
#include "CantorHost.h"
#include "DataView.h"
#include "dbmgr.h"
#include "NetworkManager.h"
#include "Forward.h"
#include "remoting.h"
#include "workermanager.h"
#include "NodeManager.h"
#include "CantorActivity.h"
#include "Config.h"
#include "xload.h"
#include "xpackage.h"
#include "CantorAPISet.h"
#include "WorkerMode.h"
#include "gvar.h"

void Test()
{

}
static X::XLoad g_xLoad;
X::Config* LoadXLangEngine(std::string& appPath,bool dbg = false)
{
    X::Config* pCfg = new X::Config();
    X::Config& cfg = *pCfg;
    cfg.appPath = appPath;
    cfg.dbg = dbg;
    cfg.enterEventLoop = true;
    cfg.runEventLoopInThread = true;
    cfg.dllSearchPath.push_back(appPath + "/../../../../../X/");
    int retCode = g_xLoad.Load(pCfg);
    if (retCode == 0)
    {
        g_xLoad.Run();
        LOG << "XLang Engine Loaded";
    }
    return pCfg;
}
void UnloadXLangEngine()
{
    g_xLoad.Unload();
}

#ifndef COMPILE_AS_SHAREDLIB
static std::string GetAppPath(char* argv0)
{
    std::string exePath0(argv0);
    std::string exePath;
    int pos = exePath0.rfind(Path_Sep);
    if (pos > 0)
    {
        exePath = exePath0.substr(0, pos);
    }
    return exePath;
}
int main(int argc, char* argv[])
{
    std::string appPath = GetAppPath(argv[0]);
    if (argc >= 2)
    {
        std::string arg1(argv[1]);
        if (arg1 == "-worker")
        {
            return WorkerMain(appPath);
        }
    }
    return CantorMain(appPath);
}
#endif
int WorkerMain(std::string& appPath)
{
    auto* pCfg = LoadXLangEngine(appPath,false);
    LOG << "Cantor-Worker Starting";
    WorkerMode::I().Loop();
    UnloadXLangEngine();
    delete pCfg;
    return 0;
}

int CantorMain(std::string& appPath,bool startNetworkServer,int port)
{
#if (WIN32)
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        return -1;
    }
#endif
    DataView::Init();
    CantorHostImpl::I().SetHostPath(appPath);
    DatabaseManager::I().SetHost(&CantorHostImpl::I());
    NetworkManager::I().SetHost(&CantorHostImpl::I());
    NodeManager::I().SetHost(&CantorHostImpl::I());
    DC::RemotingMethodProcess::I().SetHost(&CantorHostImpl::I());
    DC::WorkerManager::I().SetHost(&CantorHostImpl::I());
    GVarManager::I().Start();

    DatabaseManager::I().Start(appPath);

    std::string logFilePath = appPath + Path_Sep +"log.txt";
    Cantor::log.SetLogFileName(logFilePath);
    Cantor::log.Init();
    //SetLogLevel(-1);
    //Test();
    Config::I().SetHost(&CantorHostImpl::I());
    Config::I().Load();
    auto* pCfg = LoadXLangEngine(appPath,true);
    X::RegisterPackage<CantorAPISet>(Cantor_API_Name, &CantorAPISet::I());
    LOG << "Server::Cantor Starting";
    //MsgThread msgLoop(&CantorHostImpl::I());
    DC::RemotingMethodProcess::I().Start();
    Forwarder::I().Init(&CantorHostImpl::I());
    Forwarder::I().Start();
    LOG << "Server::Cantor Started";
    if (startNetworkServer)
    {
        NetworkManager::I().StartServer(port);
    }
    X::g_pXHost->Lrpc_Listen(0, true);
    //msgLoop.run();
    UnloadXLangEngine();
    GVarManager::I().Stop();
    delete pCfg;
    LOG << "Server::Cantor Shutdown";
    return 0;
}