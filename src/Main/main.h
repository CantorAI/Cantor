#pragma once

#include <string>

int CantorMain(std::string& appPath,bool startNetworkServer = false,int port =0);
int WorkerMain(std::string& appPath);