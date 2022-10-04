#pragma once

#include <string>
#include <vector>
#include "frame.h"

void _mkdir(const char* dir);
void ReplaceAll(std::string& data, std::string toSearch, std::string replaceStr);
std::wstring s2ws(const std::string& str);
std::string ws2s(const std::wstring& wstr);
bool exists(const std::string& name);
bool isDir(const std::string& name);
bool dir(std::string search_pat,
    std::vector<std::string>& subfolders,
    std::vector<std::string>& files);
bool IsAbsPath(std::string& path);
bool SplitPath(std::string& path, std::string& leftPath, std::string& rightPath);
void MakeOSPath(std::string& path);
std::vector<std::string> split(const std::string& str, char delim);
std::vector<std::string> split(const std::string& str, const char* delim);
bool ParsePythonFunctionCode(std::string& wholecode,std::string& funcBody);
std::string& rtrim(std::string& s);
std::string& ltrim(std::string& s);
std::string& trim(std::string& s);
unsigned long long byteStringToNumber(const char* strBytes, int size);
std::string UIDToString(UID uid);
UID UIDFromString(std::string id);
bool RunProcess(std::string cmd,
	std::string initPath,
	unsigned long& processId);

std::string GetAppName();
long long getCurMilliTimeStamp();
unsigned long GetPID();
unsigned long GetThreadID();
