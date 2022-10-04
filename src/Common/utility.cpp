#include "utility.h"
#include <cctype>
#include <codecvt>
#include <locale>
#include <sstream>
#include <vector>
#include <regex>
#include "port.h"

#if (WIN32)
#include <Windows.h>
void _mkdir(const char* dir)
{
    CreateDirectory(dir, NULL);
}
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

void _mkdir(const char* dir)
{
    int state = access(dir, R_OK | W_OK);
    if (state != 0)
    {
        mkdir(dir, S_IRWXU);
    }
}
#endif


void ReplaceAll(std::string& data, std::string toSearch, std::string replaceStr)
{
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    // Repeat till end is reached
    while (pos != std::string::npos)
    {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}
std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}
std::string ws2s(const std::wstring& wstr)
{
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;
	std::string ret = converter.to_bytes(wstr);
	return ret;
}
bool exists(const std::string& name) 
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}
bool isDir(const std::string& name)
{
	bool yes = false;
	struct stat buffer;
	if (stat(name.c_str(), &buffer) == 0)
	{
		if (buffer.st_mode & S_IFDIR)
		{
			yes = true;
		}
	}
	return yes;
}
bool IsAbsPath(std::string& path)
{
	if (path.size() == 0)
	{
		return false;
	}
	bool ret = false;
#if (WIN32)
	if ((path.find_first_of(":") !=std::string::npos) || (path[0] == '/') || (path[0] == '\\'))
	{
		ret = true;
	}
#else
	if (path[0] == '/')
	{
		ret = true;
	}
#endif
	return ret;
}
void MakeOSPath(std::string& path)
{
#if (WIN32)
	ReplaceAll(path, "/", "\\");
#else
	ReplaceAll(path, "\\", "/");
#endif
}
bool SplitPath(std::string& path, std::string& leftPath, std::string& rightPath)
{
	size_t pos = 0;
#if (WIN32)
	pos = path.find_last_of("\\/");
#else
	pos = path.find_last_of("/");
#endif
	if (pos != std::string::npos)
	{
		leftPath = path.substr(0, pos);
		rightPath = path.substr(pos + 1);
	}
	else
	{
		leftPath = "";
		rightPath = path;
	}
	return true;
}
bool dir(std::string search_pat,
	std::vector<std::string>& subfolders,
    std::vector<std::string>& files)
{
	bool ret = false;
#if (WIN32)
	BOOL result = TRUE;
	WIN32_FIND_DATA ff;

	HANDLE findhandle = FindFirstFile(search_pat.c_str(), &ff);
	if (findhandle != INVALID_HANDLE_VALUE)
	{
		ret = true;
		BOOL res = TRUE;
		while (res)
		{
			// We only want directories
			if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::string fileName(ff.cFileName);
				if (fileName != "." && fileName != "..")
				{
					subfolders.push_back(fileName);
				}
			}
			else
			{
				std::string fileName(ff.cFileName);
				files.push_back(fileName);
			}
			res = FindNextFile(findhandle, &ff);
		}
		FindClose(findhandle);
	}
#else
#include <dirent.h>
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(search_pat.c_str())) != NULL)
	{
		ret = true;
		while ((ent = readdir(dir)) != NULL) 
		{
			if (ent->d_type == DT_DIR)
			{
				std::string fileName(ent->d_name);
				if (fileName != "." && fileName != "..")
				{
					subfolders.push_back(fileName);
				}
			}
			else if (ent->d_type == DT_REG)//A regular file
			{
				std::string fileName(ent->d_name);
				files.push_back(fileName);
			}
		}
		closedir(dir);
	}
#endif
	return ret;
}
std::vector<std::string> split(const std::string& str, char delim)
{
	std::vector<std::string> list;
	std::string temp;
	std::stringstream ss(str);
	while (std::getline(ss, temp, delim))
	{
		list.push_back(temp);
	}
	return list;
}
std::vector<std::string> split(const std::string& str, const char* delim)
{
	std::vector<std::string> list;
	std::string temp;
	size_t pos = 0;
	size_t pos2 = 0;
	size_t delim_size = strlen(delim);
	while ((pos2 = str.find(delim,pos)) != std::string::npos)
	{
		temp = str.substr(pos, pos2-pos);
		list.push_back(temp);
		pos = pos2 + delim_size;
	}
	if (pos != std::string::npos)
	{
		temp = str.substr(pos);
		list.push_back(temp);
	}
	return list;
}
bool ParsePythonFunctionCode(std::string& wholecode, std::string& funcBody)
{
	bool bRet = false;
	//(?:) is non-capture group
	static std::regex rgx("\\t*def[\\t\\s]+([^\\(\\)]+)\\(([^\\\\(\\\\)]*)\\)[\\s\\t]*(?:->[\\s\\t]*([^:]+))?:");
	static std::regex rgx_var("([^:,]+)(?::([^,]+))?,?");
	//to parse def func_name([var:type]*)->var:
	std::smatch matches;
	if (std::regex_search(wholecode, matches, rgx))
	{
		if (matches.size() >= 3)
		{
			std::string body = matches.suffix();
			std::regex commentTag("\"\"\"");
			funcBody = std::regex_replace(body, commentTag, "");
			bRet = true;
		}
	}
	return bRet;
}
#define TRIMOFF_CHARS " \t\n\r\f\v"

// trim from end of string (right)
std::string& rtrim(std::string& s)
{
	s.erase(s.find_last_not_of(TRIMOFF_CHARS) + 1);
	return s;
}

// trim from beginning of string (left)
std::string& ltrim(std::string& s)
{
	s.erase(0, s.find_first_not_of(TRIMOFF_CHARS));
	return s;
}

// trim from both ends of string (right then left)
std::string& trim(std::string& s)
{
	return ltrim(rtrim(s));
}

unsigned long long byteStringToNumber(const char* strBytes,int size)//like 'xcvb'
{
	if (STRCMPNOCASE(strBytes,"any", (size>3?3:size)) ==0)
	{
		return 0;
	}
	if (size > (int)sizeof(unsigned long long))
	{
		size = sizeof(unsigned long long);
	}
	unsigned long long t = 0;
	for (int i = 0; i < size; i++)
	{
		auto c = strBytes[i];
		if (c < 256)
		{
			t = (t << 8) | c;
		}
	}
	return t;
}

UID UIDFromString(std::string id)
{
	ReplaceAll(id, "-", "");
	UID uid;
	int retVal = sscanf(id.c_str(), "%016llx%016llx", &uid.h, &uid.l);
	if (retVal != 2)
	{
		uid.h = uid.l =0;
	}
	return uid;
}

std::string UIDToString(UID uid)
{
	char id[32 + 1];
	id[32] = 0;
	snprintf(id, sizeof(id), "%016llx%016llx", uid.h, uid.l);
	return id;
}

bool RunProcess(std::string cmd,
	std::string initPath, unsigned long& processId)
{
#if (WIN32)
	STARTUPINFO StartupInfo;
	memset(&StartupInfo, 0, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION ProcessInformation;
	memset(&ProcessInformation, 0, sizeof(PROCESS_INFORMATION));
	ProcessInformation.hThread = INVALID_HANDLE_VALUE;
	ProcessInformation.hProcess = INVALID_HANDLE_VALUE;

	BOOL bOK = CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE,//if use CREATE_NEW_CONSOLE will show new cmd window
		NULL, initPath.c_str(),
		&StartupInfo, &ProcessInformation);
	if (bOK)
	{
		processId = ProcessInformation.dwProcessId;
		//::WaitForSingleObject(ProcessInformation.hProcess, -1);
	}
	if (ProcessInformation.hProcess != INVALID_HANDLE_VALUE)
	{
		CloseHandle(ProcessInformation.hProcess);
	}
	if (ProcessInformation.hThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(ProcessInformation.hThread);
	}
#else
	pid_t child_pid;
	child_pid = fork();
	if (child_pid >= 0)
	{
		if (child_pid == 0)
		{//inside child process,run the executable 
			execlp(cmd.c_str(), initPath.c_str(), (char*)nullptr);
		}
		else
		{//inside parent process, child_pid is the id for child process
			processId = child_pid;
		}
	}
#endif
	return true;
}

std::string GetAppName()
{
#if (WIN32)
	char path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	return path;
#else
	return program_invocation_name;
#endif
}
long long getCurMilliTimeStamp()
{
#if (WIN32)
	return (long long)GetTickCount64();
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

unsigned long GetPID()
{
	unsigned long processId = 0;
#if (WIN32)
	processId = GetCurrentProcessId();
#else
	processId = getpid();
#endif
	return processId;
}
unsigned long GetThreadID()
{
	unsigned long tid = 0;
#if (WIN32)
	tid = ::GetCurrentThreadId();
#else
#include <sys/types.h>
#include <unistd.h>
	tid = gettid();
#endif
	return tid;
}