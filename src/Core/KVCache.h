#pragma once

#include "singleton.h"
#include "service_def.h"
#include <unordered_map>
#include "Locker.h"
#include "xpackage.h"
#include "xlang.h"

class KVCache:
	public Singleton<KVCache>
{
public:
	KVCache();
	~KVCache();
	BEGIN_PACKAGE(KVCache)
		ADD_FUNC(2,"Set",&KVCache::Set)
		ADD_FUNC(1,"Get", &KVCache::GetAPI)
		ADD_FUNC(1,"Delete", &KVCache::Delete)
	END_PACKAGE
	bool Set(std::string key, CantorVar& var);
	CantorVar GetAPI(std::string key)
	{
		CantorVar val;
		Get(key, val);
		return val;
	}
	bool Get(std::string& key, CantorVar& var);
	bool Delete(std::string key);
private:
	Locker m_lockKV;
	std::unordered_map<std::string, CantorVar> m_kvs;
};