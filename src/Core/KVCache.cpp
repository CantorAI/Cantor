#include "KVCache.h"
#include "dbmgr.h"
#include "dbop.h"
#include "utility.h"

KVCache::KVCache()
{
}

KVCache::~KVCache()
{
}

bool KVCache::Set(std::string key, CantorVar& var)
{
	m_lockKV.Lock();
	m_kvs.emplace(std::make_pair(key, var));
	//store to db
	std::string sql;
	switch (var.GetType())
	{
	case X::ValueType::Int64:
		sql = "insert or replace into kvstore (name,value1) values (?,?)";
		break;
	case X::ValueType::Double:
		sql = "insert or replace into kvstore (name,value2) values (?,?)";
		break;
	case X::ValueType::Str:
		sql = "insert or replace into kvstore (name,value3) values (?,?)";
		break;
	case X::ValueType::Object:
	{
		auto* pObj = var.GetObj();
		if (pObj->GetType() == X::ObjType::Str)
		{
			sql = "insert or replace into kvstore (name,value3) values (?,?)";
		}
	}
		break;
	default:
		break;
	}
	DatabaseManager::I().BeginTransaction();
	DBStatement stat(sql.c_str());
	stat.bindtext(1, s2ws(key));
	switch (var.GetType())
	{
	case X::ValueType::Int64:
		stat.bindint64(2, var.GetLongLong());
		break;
	case X::ValueType::Double:
		stat.binddouble(2,var.GetDouble());
		break;
	case X::ValueType::Str:
		stat.bindtext(2, s2ws(var.ToString()));
		break;
	case X::ValueType::Object:
	{
		auto* pObj = var.GetObj();
		if (pObj->GetType() == X::ObjType::Str)
		{
			stat.bindtext(2, s2ws(var.ToString()));
		}
	}
		break;
	default:
		break;
	}
	stat.step();
	DatabaseManager::I().EndTransaction();
	m_lockKV.Unlock();
	return true;
}

bool KVCache::Get(std::string& key, CantorVar& var)
{
	bool bHave = false;
	m_lockKV.Lock();
	auto it = m_kvs.find(key);
	if (it != m_kvs.end())
	{
		var = it->second;
		bHave = true;
	}
	m_lockKV.Unlock();
	if (!bHave)
	{//try to load from database
		DBStatement stat("select value1,value2,value3 from kvstore where name =?");
		stat.bindtext(1, s2ws(key));
		if (stat.step() == DBState::Row)
		{
			if (!stat.isNull(0))
			{
				var =  stat.getInt64Value(0);
				bHave = true;
			}
			else if (!stat.isNull(1))
			{
				var = stat.getDouble(1);
				bHave = true;
			}
			else if (!stat.isNull(2))
			{
				std::string strVal;
				stat.getValue(2, strVal);
				var = strVal;
				bHave = true;
			}
			else
			{//Null
				var = X::Value();
			}
		}
		if (bHave)
		{
			m_lockKV.Lock();
			m_kvs.emplace(std::make_pair(key, var));
			m_lockKV.Unlock();
		}
	}
	return bHave;
}

bool KVCache::Delete(std::string key)
{
	bool bHave = false;
	m_lockKV.Lock();
	auto it = m_kvs.find(key);
	if (it != m_kvs.end())
	{
		m_kvs.erase(it);
		bHave = true;
	}
	DatabaseManager::I().BeginTransaction();
	DBStatement stat("delete kvstore where name=?");
	stat.bindtext(1, s2ws(key));
	DatabaseManager::I().EndTransaction();

	m_lockKV.Unlock();

	return bHave;
}
