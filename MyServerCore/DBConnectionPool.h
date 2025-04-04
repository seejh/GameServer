#pragma once
#include"DBConnection.h"

/*
	// SQL Handle Environment
	DB Connection Pool
*/
class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	bool			Connect(int32 connectionCount, const WCHAR* connectionString);
	bool			ConnectA(int32 connectionCount, const CHAR* connectionString);
	void			Clear();


	DBConnection*	Pop();

	void			Push(DBConnection* connection);

private:
	
	mutex _mutex;
	SQLHENV					_environment = SQL_NULL_HANDLE;
	vector<DBConnection*>	_connections;
};
