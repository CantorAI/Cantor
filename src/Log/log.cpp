#include "log.h"
#include "gxydef.h"
#include "utility.h"

Cantor::Log Cantor::log;

Cantor::Log::Log()
{
}

Cantor::Log::~Log()
{
	m_file_log.close();
}

bool Cantor::Log::Init()
{
	m_file_log.open(m_logFileName, std::ios_base::out);
	return true;
}

Cantor::Log& Cantor::Log::SetCurInfo(const char* fileName,
	const int line, const int level)
{
	m_level = level;
	if (m_level <= m_dumpLevel)
	{
		std::string strFileName(fileName);
		auto pos = strFileName.rfind(Path_Sep_S);
		if (pos != std::string::npos)
		{
			strFileName = strFileName.substr(pos + 1);
		}
		unsigned long pid = GetPID();
		unsigned long tid = GetThreadID();
		int64_t ts = getCurMilliTimeStamp();
		LOG_OUT("\n[pid:");
		LOG_OUT(pid);
		LOG_OUT(",tid:");
		LOG_OUT(tid);
		LOG_OUT(",ts:");
		LOG_OUT(ts); LOG_OUT(',');
		LOG_OUT(strFileName);
		LOG_OUT(":"); LOG_OUT(line); LOG_OUT("]");
	}
	return *this;
}
