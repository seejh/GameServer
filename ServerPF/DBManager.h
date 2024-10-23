#pragma once

#include"DBDataModel.h"

class GameDBManager;
class SharedDBManager;
class RedisManager;
class Player;
class ClientSession;


/*
	DB 종류가 많음. 한곳에서 접근 관리(DBManager)
	서버 로직과 DB 분리는 나중에 구현
	
	DBManager(static instance)를 통해 다른 DB들 접근
*/
class DBManager
{
public:
	//
	static DBManager* Instance() {
		static DBManager instance;
		return &instance;
	}

	void Init();
	
public:
	shared_ptr<GameDBManager> _gameDbManager;
	shared_ptr<SharedDBManager> _sharedDbManager;
	shared_ptr<RedisManager> _redisDbManager;
};

