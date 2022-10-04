#pragma once

#include <iostream>
#include <fstream>
#include <sstream> 
#include <string>


namespace Cantor {
class Log
{
#define LOG_OUT(v)\
	if (m_toFile)\
	{\
		m_file_log << v;\
	}\
	if (m_toStdOut) \
	{\
		std::cout << v;\
	}

#define LOG_FLUSH()\
	if(m_toFile) m_file_log.flush();
public:
	Log();
	~Log();

	bool Init();
	template<typename T>
	Log& operator<<(T v)
	{
		if (m_level <= m_dumpLevel)
		{
			LOG_OUT(v);
			LOG_FLUSH();
		}
		return *this;
	}
	void SetLogFileName(std::string& strFileName)
	{
		m_logFileName = strFileName;
	}
	Log& SetCurInfo(const char* fileName, const int line,const int level);

	void SetDumpLevel(int l)
	{
		m_dumpLevel = l;
	}
private:
	bool m_toFile = false;
	bool m_toStdOut = true;
	std::string m_logFileName;
	std::ofstream m_file_log;

	int m_level = 0;
	int m_dumpLevel = 999999;
};
extern Log log;
#define SetLogLevel(l) Cantor::log.SetDumpLevel(l)
#define LOGV(level) Cantor::log.SetCurInfo(__FILE__,__LINE__,level)
#define LOG LOGV(0)
#define LOG1 LOGV(1)
#define LOG2 LOGV(2)
#define LOG3 LOGV(3)
#define LOG4 LOGV(4)
#define LOG5 LOGV(5)
#define LOG6 LOGV(6)
#define LOG7 LOGV(7)
#define LOG8 LOGV(8)
#define LOG9 LOGV(9)

}