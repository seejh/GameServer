#include "pch.h"
#include "SessionManager.h"


#include"ClientSession.h"

// #include"DBDataModel.h"
#include"DBManager.h"
#include"GameDBManager.h"
#include"RedisManager.h"
#include"JwtHandler.h"
#include"ClientPacketHandler.h"

int32 sessionIdGen = 0;

void SessionManager::Init()
{
	FlushSend();
}

// 잡큐 없고 내부적으로 락을 들고 있을 때
void SessionManager::AddSession(shared_ptr<ClientSession> session)
{
	// lock_guard<mutex> lock(_mutex);

	session->_sessionId = sessionIdGen++;

	_sessions.insert(session);
}

void SessionManager::RemoveSession(shared_ptr<ClientSession> session)
{
	// lock_guard<mutex> lock(_mutex);
	_sessions.erase(session);
}

int32 SessionManager::GetSessionCount()
{
	// lock_guard<mutex> lock(_mutex);
	return _sessions.size();
}


void SessionManager::FlushSend()
{
	// lock_guard<mutex> lock(_mutex);

	for (shared_ptr<ClientSession> s : _sessions) 
		s->SendPacketFlush();

	// 
	uint64 tickAfter = 200;
	DoTimer(tickAfter, &SessionManager::FlushSend);
}

