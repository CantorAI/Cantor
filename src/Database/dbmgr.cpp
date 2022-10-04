#include "dbmgr.h"
#include "CantorHost.h"
#include "sqlite/sqlite3.h"
#include "utility.h"
#include "dbop.h"

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
}
#if 0
static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}
#endif
void DatabaseManager::Start(std::string& exePath)
{
	mExePath = exePath;

	sqlite3* db = nullptr;
	int rc;

	std::string dbName = mExePath + "/cantorstore.db";
	rc = sqlite3_open(dbName.c_str(), &db);
	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
	}
	else
	{
		mdb = db;
	}
	BuildTables();
}

void DatabaseManager::Close()
{
	if (mdb)
	{
		sqlite3_close(mdb);
		mdb = nullptr;
	}
}
int exec_callback(void* NotUsed, int argc, char** argv, char** azColName)
{
	return 0;
}
void DatabaseManager::BeginTransaction()
{
	ExecSQL(L"BEGIN TRANSACTION;");
}
void DatabaseManager::EndTransaction()
{
	ExecSQL(L"END TRANSACTION;");
}
int DatabaseManager::ExecSQL(std::wstring sql)
{
	auto aSql = ws2s(sql);
	char* zErrMsg = 0;
	int rc = sqlite3_exec(mdb, aSql.c_str(), exec_callback, 0, &zErrMsg);
	return rc;
}

bool DatabaseManager::CheckTableExist(std::wstring tableName)
{
	bool bHave = false;
	DBStatement stat("SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = ?");
	stat.bindtext(1, tableName);
	if (stat.step() == DBState::Row)
	{
		int cnt = stat.getValue(0);
		bHave = (cnt > 0);
	}
	return bHave;
}

void DatabaseManager::BuildTables()
{
	//TaskCache
	if (!CheckTableExist(L"TaskCache"))
	{
		ExecSQL(L"CREATE TABLE \"TaskCache\" (\
			\"TaskId\"	TEXT,\
			\"TaskContent\"	BLOB,\
			PRIMARY KEY(\"TaskId\")\
			)"
		);
	}
	//kvstore
	if (!CheckTableExist(L"kvstore"))
	{
		ExecSQL(L"CREATE TABLE \"kvstore\" (\
			\"name\"	TEXT UNIQUE,\
			\"value1\"	INTEGER,\
			\"value2\"	REAL,\
			\"value3\"	TEXT,\
			PRIMARY KEY(\"name\")\
			)"
		);
	}
	//running_pipeline
	if (!CheckTableExist(L"running_pipeline"))
	{
		ExecSQL(L"CREATE TABLE \"running_pipeline\" (\
				\"InstanceId\"	TEXT,\
				\"Desc\"	TEXT,\
				PRIMARY KEY(\"InstanceId\")\
				)"
		);
	}
}
