#pragma once
#include "singleton.h"
#include <string>

struct sqlite3;
class CantorHost;
class DatabaseManager :
    public Singleton<DatabaseManager>
{
public:
    DatabaseManager();
    ~DatabaseManager();

    void SetHost(CantorHost* p)
    {
        mHost = p;
    }
    void Start(std::string& exePath);
    void Close();
    sqlite3* db()
    {
        return mdb;
    }
    void BeginTransaction();
    void EndTransaction();
    int ExecSQL(std::wstring sql);
private:
    bool CheckTableExist(std::wstring tableName);
    void BuildTables();
    std::string mExePath;
    CantorHost* mHost = nullptr;
    sqlite3* mdb = nullptr;
};