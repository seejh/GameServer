#pragma once


// DBConnectionPool ��ü�� ���������� ���� ���
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

