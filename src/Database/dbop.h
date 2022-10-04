#pragma once
#include <string>

enum class DBState
{
	Ok = 0,//SQLITE_OK
	Row = 100,//SQLITE_ROW
	Done = 101,//SQLITE_DONE
};

struct sqlite3;
struct sqlite3_stmt;

class DBStatement
{
public:
	DBStatement();
	DBStatement(const char* str);
	~DBStatement();
	bool bindtext(int idx, std::wstring str);
	bool bindblob(int idx, const char* pData,int nData);
	bool bindint(int idx, int val);
	bool binddouble(int idx, double val);
	bool bindint64(int idx, long long val);
	int getcolnum();
	DBState step();
	void reset();
	bool getValue(int idx, std::wstring& val);
	bool getValue(int idx, std::string& val);
	bool blob2text(const void* blob, int size, std::wstring& val);
	std::wstring getColName(int idx);
	int getValue(int idx);
	long long getInt64Value(int idx);
	double getDouble(int idx);
	bool isNull(int idx);
	const void* getBlobValue(int idx);
	int getBlobSize(int idx);
	int SC()
	{
		return statecode;
	}
private:
	sqlite3_stmt* stmt = NULL;
	int statecode = 0;
};
