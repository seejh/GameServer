#include"pch.h"
#include "SharedDBManager.h"

#include<DBConnection.h>
#include<DBConnectionPool.h>
#include"GenSharedDBProcedures.h"
#include"GameSessionManager.h"

#include"ConfigManager.h"

bool SharedDBManager::Connect(int connectionCount, const WCHAR* connectionString)
{
	if (_dbConnPool == nullptr)
		_dbConnPool = new DBConnectionPool();

	_connectionCounts = connectionCount + 1;

	if (_dbConnPool->Connect(_connectionCounts, connectionString) == false) {
		cout << "[SharedDBManager] Connect Error - Connect Failed" << endl;
		return false;
	}

	_dbConn = Pop();
	if (_dbConn == nullptr) {
		cout << "[SharedDBManager] Connect Error - _dbConn Nullptr" << endl;
		return false;
	}
	
	//DoTimer(GetTickCount64() + (15 * 1000), [this]() {SaveServerInfo(); });

	cout << "[SharedDbManager] Connect OK" << endl;

	return true;
}

bool SharedDBManager::ConnectA(int connectionCount, const CHAR* connectionString)
{
	if (_dbConnPool == nullptr)
		_dbConnPool = new DBConnectionPool();

	_connectionCounts = connectionCount + 1;

	if (_dbConnPool->ConnectA(_connectionCounts, connectionString) == false) {
		cout << "[SharedDBManager] ConnectA Error - Connect Failed" << endl;
		return false;
	}

	_dbConn = Pop();
	if (_dbConn == nullptr) {
		cout << "[SharedDBManager] ConnectA Error - _dbConn Nullptr" << endl;
		return false;
	}

	cout << "[SharedDBManager] Connect OK" << endl;

	return true;
}

DBConnection* SharedDBManager::Pop()
{
	return _dbConnPool->Pop();
}

void SharedDBManager::Push(DBConnection* dbConn)
{
	_dbConnPool->Push(dbConn);
}

void SharedDBManager::UpdateServerInfo()
{
	// 일단 여기다가 달아놓는 걸로

	// 서버정보 & 현 세션 개수
	int32 sessionCount = GameSessionManager::Instance()->GetSessionCount();

	// 
	ServerInfoDB infoDb;
	ServerConfig serverConfig;
	ConfigManager::Instance()->LoadConfigByName("GameServer", &serverConfig);

	infoDb.ServerDbId = serverConfig._serverDbId;
	// str -> wstr
	wcscpy_s(infoDb.Name, Config::StrToWstr(serverConfig._serverName).c_str());
	wcscpy_s(infoDb.IpAddress, Config::StrToWstr(serverConfig._ip).c_str());
	infoDb.Port = serverConfig._port;
	infoDb.BusyScore = sessionCount;

	// 서버 정보 업데이트
	SP::UpdateServerInfo updateServerDb(*_dbConn);
	updateServerDb.In_ServerDbId(infoDb.ServerDbId);
	updateServerDb.In_Name(infoDb.Name);
	updateServerDb.In_IpAddress(infoDb.IpAddress);
	updateServerDb.In_Port(infoDb.Port);
	updateServerDb.In_BusyScore(infoDb.BusyScore);
	if (updateServerDb.Execute() == false) {
		cout << "SharedDbManager::SaveServerInfo() Error - Execute SP" << endl;
		return;
	}

	// 다음 작업 예약 (15초)
	uint64 tickAfter = 1000 * 5;

	DoTimer(tickAfter, &SharedDBManager::UpdateServerInfo);
}


