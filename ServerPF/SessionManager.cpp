#include "pch.h"
#include "SessionManager.h"


#include"ClientSession.h"


void SessionManager::AddSession(shared_ptr<ClientSession> session)
{
	lock_guard<mutex> lock(_mutex);
	_sessions.insert(session);
}

void SessionManager::RemoveSession(shared_ptr<ClientSession> session)
{
	lock_guard<mutex> lock(_mutex);
	_sessions.erase(session);
}


void SessionManager::FlushSend()
{
	lock_guard<mutex> lock(_mutex);

	for (shared_ptr<ClientSession> s : _sessions) 
		s->SendPacketFlush();
}

