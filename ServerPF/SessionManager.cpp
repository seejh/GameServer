#include "pch.h"
#include "SessionManager.h"


#include"ClientSession.h"

#include"DBDataModel.h"
#include"GameDBManager.h"

// ��ť ���� ���������� ���� ��� ���� ��
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

int32 SessionManager::GetSessionCount()
{
	lock_guard<mutex> lock(_mutex);
	return _sessions.size();
}


void SessionManager::FlushSend()
{
	lock_guard<mutex> lock(_mutex);

	for (shared_ptr<ClientSession> s : _sessions) 
		s->SendPacketFlush();
}

