#include "pch.h"
#include "DBManager.h"

#include"GameDBManager.h"
#include"SharedDBManager.h"
#include"GameObject.h"
#include"Item.h"
#include"GenProcedures.h"
#include"GenSharedDBProcedures.h"
#include"GameRoom.h"
#include"GameClientPacketHandler.h"
#include"RedisManager.h"
#include"GameSession.h"


bool DBManager::Init(int32 gameConnCounts, const WCHAR* gameConnString, int32 sharedConnCounts, const WCHAR* sharedConnString, int32 redisConnCounts, const char* redisConnString, int32 redisPort)
{
	// Game DB
	_gameDbManager = make_shared<GameDBManager>();
	if (_gameDbManager->Connect(gameConnCounts, gameConnString) == false)
		return false;

	// Shared DB
	_sharedDbManager = make_shared<SharedDBManager>();
	if (_sharedDbManager->Connect(sharedConnCounts, sharedConnString) == false)
		return false;

	// Redis
	_redisDbManager = make_shared<RedisDBManager>();
	if (_redisDbManager->Connect(redisConnString, redisPort, redisConnCounts) == false)
		return false;

	return true;
}

bool DBManager::InitA(int32 gameConnCounts, const CHAR* gameConnString, int32 sharedConnCounts, const CHAR* sharedConnString, int32 redisConnCounts, const char* redisConnString, int32 redisPort)
{
	// Game DB
	_gameDbManager = make_shared<GameDBManager>();
	if (_gameDbManager->ConnectA(gameConnCounts, gameConnString) == false)
		return false;

	// Shared DB
	_sharedDbManager = make_shared<SharedDBManager>();
	if (_sharedDbManager->ConnectA(sharedConnCounts, sharedConnString) == false)
		return false;

	// Redis
	_redisDbManager = make_shared<RedisDBManager>();
	if (_redisDbManager->Connect(redisConnString, redisPort, redisConnCounts) == false)
		return false;

	return true;
}
