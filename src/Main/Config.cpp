#include "Config.h"
#include "CantorHost.h"
#include "gxydef.h"
#include "utility.h"
#include "workermanager.h"
#include "RegistryClient.h"
#include "NetworkManager.h"
#include "log.h"
#include "RegistryServer.h"

#define CONFIG_FILE_NAME "config"
bool Config::Load()
{
#if _TODO_
	auto hostPath = CantorHostImpl::I().GetHostPath();
	std::string cfgFile;
	cfgFile = hostPath + Path_Sep + "Config" + Path_Sep+ CONFIG_FILE_NAME + ".yaml";
	if (!exists(cfgFile))
	{
		cfgFile = hostPath + Path_Sep + "Config" + Path_Sep + CONFIG_FILE_NAME + ".json";
		if (!exists(cfgFile))
		{
			cfgFile = hostPath + Path_Sep +Path_Sep + CONFIG_FILE_NAME + ".yaml";
			if (!exists(cfgFile))
			{
				cfgFile = hostPath + Path_Sep + Path_Sep + CONFIG_FILE_NAME + ".json";
				if (!exists(cfgFile))
				{
					return false;
				}
			}
		}
	}
	SuperNode::Node* root = SuperNode::LoadFromYamlFile(cfgFile.c_str());
	if (root == nullptr)
	{
		return false;
	}
	SuperNode::Node& Root = *(SuperNode::Node*)root;
	SuperNode::Node& AutoRun = Root["AutoRun"];
	if (AutoRun.IsSequence())
	{
		for (int i = 0; i < AutoRun.Count(); i++)
		{
			std::string runFile = AutoRun[i];
			if (runFile.rfind(".py") == runFile.size()-3)
			{//python
				runFile = hostPath + Path_Sep + runFile;
				std::string cmd = "python "+ runFile;
				unsigned long processId = 0;
				RunProcess(cmd, hostPath, processId);
			}
		}
	}
	//RegistryServer
	SuperNode::Node& RegistryServer = Root["RegistryServer"];
	if (RegistryServer.IsMap())
	{
		bool bEnable = RegistryServer["Enable"];
		if (bEnable)
		{
			int port = RegistryServer["Port"];
			if (port != 0)
			{
				bool bOK = NetworkManager::I().StartServer(port);
				if (bOK)
				{
					RegistryServer::I().SetHost(mHost);
					RegistryServer::I().Start();
					LOG << "Started as Registry Server with port:" << port;
				}
				else
				{
					LOG << "Failed starting Registry Server with port:" << port;
				}
			}
		}
	}
	//PublishToRegistry
	SuperNode::Node& PublishToRegistry = Root["PublishToRegistry"];
	if (PublishToRegistry.IsMap())
	{
		bool bEnable = PublishToRegistry["Enable"];
		if (bEnable)
		{
			RegistryClient::I().SetHost(mHost);
			SuperNode::Node& Servers = PublishToRegistry["Servers"];
			if (Servers.IsSequence())
			{
				int cnt = Servers.Count();
				for (int i = 0; i < cnt; i++)
				{
					SuperNode::Node& srvObj = Servers[i];
					std::string Server_Address = srvObj["Server_Address"];
					int Server_Port = srvObj["Server_Port"];
					RegistryClient::I().AddRegistryServer(Server_Address, Server_Port);
				}
				if (cnt > 0)
				{//start thread to connect to registry
					RegistryClient::I().Start();
				}
			}
		}
	}
#endif
	return true;
}
