#pragma once

#include"IocpCore.h"
#include"Session.h"
//#include"ListenSession.h"

class ListenSession;
enum class ServiceType : uint8 {
	SERVER, CLIENT,
};

class NetService
{
public:
	NetService(ServiceType type, function<shared_ptr<Session>()> sessionFactory, string ip, int port, int maxSessionCounts);
	virtual ~NetService();

	virtual bool Init() abstract;

	shared_ptr<Session> CreateSession();
	void AddSession(shared_ptr<Session> session);
	void RemoveSession(shared_ptr<Session> session);
	void Broadcast(shared_ptr<SendBuffer> sendBuffer);

public:
	shared_ptr<IocpCore> _iocpCore;
	shared_ptr<ListenSession> _listenSession;
	SOCKADDR_IN _sockAddr;

	set<shared_ptr<Session>> _sessions;
	int _connectedSessionCount = 0;
	int _maxSessionCounts;

	function<shared_ptr<Session>()> _sessionFactory;
	
	ServiceType _serviceType;
	string _ip;
	int _port;
	mutex _mutex;
};

class ClientService : public NetService {
public:
	ClientService(function<shared_ptr<Session>()> sessionFactory, string ip, int port, int maxSessionCounts);
	virtual ~ClientService() {}

	virtual bool Init() override;

};

class ServerService : public NetService {
public:
	ServerService(function<shared_ptr<Session>()> sessionFactory, string ip, int port, int maxSessionCounts);
	virtual ~ServerService() {}

	virtual bool Init() override;
};