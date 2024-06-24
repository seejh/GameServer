#include "pch.h"
#include "NetService.h"




NetService::NetService(function<shared_ptr<Session>()> sessionFactory) : _sessionFactory(sessionFactory)
{
}

NetService::~NetService()
{
}

bool NetService::Init()
{
	_iocpCore = make_shared<IocpCore>();
	_listenSession = make_shared<ListenSession>(this);
	if (_listenSession->Init())
		return false;

	cout << "Listen Init OK now Listening..." << endl;

	return true;
}

shared_ptr<Session> NetService::CreateSession()
{
	return _sessionFactory();
}

void NetService::AddSession(shared_ptr<Session> session)
{
	lock_guard<mutex> _lock(_mutex);
	_sessions.insert(session);
	_connectedSessionCount++;
}

void NetService::RemoveSession(shared_ptr<Session> session)
{
	lock_guard<mutex> _lock(_mutex);
	ASSERT_CRASH(_sessions.erase(session) != 0);
	_connectedSessionCount--;
}

void NetService::Broadcast(shared_ptr<SendBuffer> sendBuffer)
{
	lock_guard<mutex> _lock(_mutex);
	for (auto session : _sessions) {
		session->Send(sendBuffer);
	}
}

