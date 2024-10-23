#pragma once


// DBConnectionPool 객체가 내부적으로 락을 사용
class DBConnection;
class SharedDBManager //: public JobQueue
{
public:
	bool			Connect(int connectionCount, const WCHAR* connectionString);
	// DBConnection*	GetConn();
	DBConnection*	Pop();
	void			Push(DBConnection* dbConn);

	//
	void SaveServerInfo();

public:
	int32 _connectionCounts;
	DBConnection* _dbConn;
	DBConnectionPool* _dbConnectionPool;
};

