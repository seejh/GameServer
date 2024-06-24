#pragma once

#include"IocpCore.h"
#include"Session.h"
#include"ListenSession.h"

class NetService
{
public:
	NetService(function<shared_ptr<Session>()> sessionFactory);
	~NetService();

	bool Init();

	shared_ptr<Session> CreateSession();
	void AddSession(shared_ptr<Session> session);
	void RemoveSession(shared_ptr<Session> session);
	void Broadcast(shared_ptr<SendBuffer> sendBuffer);

public:
	shared_ptr<IocpCore> _iocpCore;
	shared_ptr<ListenSession> _listenSession;

	set<shared_ptr<Session>> _sessions;
	int _connectedSessionCount = 0;

	function<shared_ptr<Session>()> _sessionFactory;
	
	mutex _mutex;
};

