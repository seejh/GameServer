#pragma once

#include"DBDataModel.h"

// DBConnectionPool 객체 내부 락 사용
class DBConnection;
class SharedDBManager : public JobQueue
{
public:
	bool			Connect(int connectionCount, const WCHAR* connectionString);
	
	DBConnection*	Pop();
	void			Push(DBConnection* dbConn);

	// 일단 여기다가 달아놓는걸로
	void UpdateServerInfo(/*ServerInfoDB serverInfoDB*/);

public:
	DBConnection* _dbConn;
	DBConnectionPool* _dbConnPool;

	int32 _connectionCounts;
};

