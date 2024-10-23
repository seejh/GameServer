#include "pch.h"
#include "DBManager.h"

#include"GameDBManager.h"
#include"SharedDBManager.h"
#include"GameObject.h"
#include"Item.h"
#include"GenProcedures.h"
#include"GenSharedDBProcedures.h"
#include"Room.h"
#include"ClientPacketHandler.h"
#include"RedisManager.h"
#include"ClientSession.h"

void DBManager::Init() {
	_gameDbManager = make_shared<GameDBManager>();
	_sharedDbManager = make_shared<SharedDBManager>();
}


