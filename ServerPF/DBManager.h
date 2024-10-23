#pragma once

#include"DBDataModel.h"

class GameDBManager;
class SharedDBManager;
class RedisManager;
class Player;
class ClientSession;


/*
	DB ������ ����. �Ѱ����� ���� ����(DBManager)
	���� ������ DB �и��� ���߿� ����
	
	DBManager(static instance)�� ���� �ٸ� DB�� ����
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

