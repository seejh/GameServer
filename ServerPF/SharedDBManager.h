#pragma once

#include"DBDataModel.h"

// DBConnectionPool ��ü ���� �� ���
class DBConnection;
class SharedDBManager : public JobQueue
{
public:
	bool			Connect(int connectionCount, const WCHAR* connectionString);
	
	DBConnection*	Pop();
	void			Push(DBConnection* dbConn);

	// �ϴ� ����ٰ� �޾Ƴ��°ɷ�
	void UpdateServerInfo(/*ServerInfoDB serverInfoDB*/);

public:
	DBConnection* _dbConn;
	DBConnectionPool* _dbConnPool;

	int32 _connectionCounts;
};

