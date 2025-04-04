#pragma once

#include"DBDataModel.h"

class GameDBManager;
class SharedDBManager;
class RedisDBManager;
class Player;
class GameSession;


/*--------------------------------------------------------
	DB 종류가 많음. 한곳에서 접근 관리(DBManager)
---------------------------------------------------------*/
class DBManager
{
public:
	//
	static DBManager* Instance() {
		static DBManager instance;
		return &instance;
	}

	bool Init(int32 gameConnCounts, const WCHAR* gameConnString,
		int32 sharedConnCounts, const WCHAR* sharedConnString,
		int32 redisConnCounts, const char* redisConnString, int32 redisPort);

	bool InitA(int32 gameConnCounts, const CHAR* gameConnString,
		int32 sharedConnCounts, const CHAR* sharedConnString,
		int32 redisConnCounts, const char* redisConnString, int32 redisPort);


public:
	shared_ptr<GameDBManager> _gameDbManager;
	shared_ptr<SharedDBManager> _sharedDbManager;
	shared_ptr<RedisDBManager> _redisDbManager;
};

