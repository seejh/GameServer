#include "pch.h"
#include "GameSessionManager.h"


#include"GameSession.h"

// #include"DBDataModel.h"
#include"DBManager.h"
#include"GameDBManager.h"
#include"RedisManager.h"
#include"JwtHandler.h"
#include"GameClientPacketHandler.h"

int32 sessionIdGen = 0;

void GameSessionManager::Init()
{
	FlushSend();
}

// 잡큐 없고 내부적으로 락을 들고 있을 때
void GameSessionManager::AddSession(shared_ptr<GameSession> session)
{
	// lock_guard<mutex> lock(_mutex);

	session->_sessionId = sessionIdGen++;

	_sessions.insert(session);
}

void GameSessionManager::RemoveSession(shared_ptr<GameSession> session)
{
	// lock_guard<mutex> lock(_mutex);
	_sessions.erase(session);
}

int32 GameSessionManager::GetSessionCount()
{
	// lock_guard<mutex> lock(_mutex);
	return _sessions.size();
}


void GameSessionManager::FlushSend()
{
	// lock_guard<mutex> lock(_mutex);

	for (shared_ptr<GameSession> s : _sessions) 
		s->SendPacketFlush();

	// 
	uint64 tickAfter = 200;
	DoTimer(tickAfter, &GameSessionManager::FlushSend);
}

