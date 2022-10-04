#include "dbop.h"
#include "dbmgr.h"
#include "utility.h"
#include "sqlite/sqlite3.h"

DBStatement::DBStatement()
{
}

DBStatement::DBStatement(const char* strSql)
{
	sqlite3* pdb = DatabaseManager::I().db();
	if (pdb)
	{
		statecode = sqlite3_prepare(
			pdb,
			strSql,
			-1,
			&stmt,
			0  // Pointer to unused portion of stmt
		);
	}
	else
	{
		statecode = SQLITE_NOTFOUND;
	}
}

DBStatement::~DBStatement()
{
	if (stmt)
	{
		sqlite3_finalize(stmt);
		stmt = nullptr;
	}
}

bool DBStatement::bindtext(int idx, std::wstring str)
{
	if (statecode != SQLITE_OK)
	{
		return false;
	}
	std::string utf8str = ws2s(str);
	statecode = sqlite3_bind_text(
		stmt,
		idx,  // Index of wildcard
		utf8str.c_str(),
		utf8str.length(),  // length of text
		SQLITE_TRANSIENT
	);
	return (statecode == SQLITE_OK);
}

bool DBStatement::bindblob(int idx, const char* pData, int nData)
{
	if (statecode != SQLITE_OK)
	{
		return false;
	}
	statecode = sqlite3_bind_blob(
		stmt,
		idx,
		(const void*)pData,
		nData, SQLITE_TRANSIENT
	);
	return (statecode == SQLITE_OK);
}

bool DBStatement::bindint(int idx, int val)
{
	if (statecode != SQLITE_OK)
	{
		return false;
	}
	statecode = sqlite3_bind_int(
		stmt,
		idx,
		val
	);
	return (statecode == SQLITE_OK);
}
bool DBStatement::binddouble(int idx, double val)
{
	if (statecode != SQLITE_OK)
	{
		return false;
	}
	statecode = sqlite3_bind_double(
		stmt,
		idx,
		val
	);
	return (statecode == SQLITE_OK);
}
bool DBStatement::bindint64(int idx, long long val)
{
	if (statecode != SQLITE_OK)
	{
		return false;
	}
	statecode = sqlite3_bind_int64(
		stmt,
		idx,
		val
	);
	return (statecode == SQLITE_OK);
}
int DBStatement::getcolnum()
{
	return sqlite3_column_count(stmt);
}

DBState DBStatement::step()
{
	statecode = sqlite3_step(stmt);
	return (DBState)statecode;
}

void DBStatement::reset()
{
	sqlite3_reset(stmt);
}

bool DBStatement::getValue(int idx, std::wstring& val)
{
	const char* retstr = NULL;
	retstr = (const char*)sqlite3_column_text(stmt, idx);
	if (retstr == NULL)
	{
		return false;
	}
	std::string txt((const char*)retstr);
	val = s2ws(txt);
	return true;
}

bool DBStatement::getValue(int idx, std::string& val)
{
	const char* retstr = NULL;
	retstr = (const char*)sqlite3_column_text(stmt, idx);
	if (retstr == NULL)
	{
		return false;
	}
	val = retstr;
	return true;
}

bool DBStatement::blob2text(const void* blob,
	int size, std::wstring& val)
{
	std::string txt((const char*)blob, size);
	val = s2ws(txt);
	return true;
}

const void* DBStatement::getBlobValue(int idx)
{
	return sqlite3_column_blob(stmt, idx);
}

int DBStatement::getBlobSize(int idx)
{
	return sqlite3_column_bytes(stmt, idx);
}

std::wstring DBStatement::getColName(int idx)
{
	const char* name = NULL;
	name = (const char*)sqlite3_column_name(
		stmt, idx);
	if (name == NULL)
	{
		return L"";
	}
	std::string txt((const char*)name);
	auto val = s2ws(txt);
	return val;
}

int DBStatement::getValue(int idx)
{
	return sqlite3_column_int(stmt, idx);
}

long long DBStatement::getInt64Value(int idx)
{
	return sqlite3_column_int64(stmt,idx);
}

double DBStatement::getDouble(int idx)
{
	return sqlite3_column_double(stmt, idx);
}

bool DBStatement::isNull(int idx)
{
	return sqlite3_column_type(stmt,idx) == SQLITE_NULL;
}
