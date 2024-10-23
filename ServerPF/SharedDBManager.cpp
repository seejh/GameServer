#include"pch.h"
#include "SharedDBManager.h"

#include<DBConnection.h>
#include<DBConnectionPool.h>

#include"ConfigManager.h"
#include"SessionManager.h"
#include"DBDataModel.h"
#include"GenSharedDBProcedures.h"

bool SharedDBManager::Connect(int connectionCount, const WCHAR* connectionString)
{
	_connectionCounts = connectionCount + 1;
	_dbConnectionPool = new DBConnectionPool();

	if (_dbConnectionPool->Connect(_connectionCounts, connectionString) == false)
		return false;

	_dbConn = Pop();
	if (_dbConn == nullptr)
		return false;

	// 여기서 빼야된다.
	//DoTimer(GetTickCount64() + (15 * 1000), [this]() {SaveServerInfo(); });

	return true;
}

DBConnection* SharedDBManager::Pop()
{
	return _dbConnectionPool->Pop();
}

void SharedDBManager::Push(DBConnection* dbConn)
{
	_dbConnectionPool->Push(dbConn);
}

void SharedDBManager::SaveServerInfo()
{
	// 서버정보 & 현 세션 개수
	ServerConfig config = ConfigManager::Instance()->_config;
	int32 sessionCount = SessionManager::Instance()->GetSessionCount();
	
	// 
	ServerInfoDB infoDb;
	infoDb.ServerDbId = config.ServerDbId;
	wcscpy_s(infoDb.Name, config.Name.c_str());
	wcscpy_s(infoDb.IpAddress, config.IpAddress.c_str());
	infoDb.Port = config.Port;
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
	uint64 tickAfter = GetTickCount64() + (1000 * 15);
	//shared_ptr<Job> reservedJob = DoTimer(tickAfter, [this]() { SaveServerInfo(); });
}
