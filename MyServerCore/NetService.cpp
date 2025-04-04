#include "pch.h"
#include "NetService.h"
#include"ListenSession.h"

/*------------------------------------------------------------------------------------------
	Service
-------------------------------------------------------------------------------------------*/

NetService::NetService(ServiceType type, function<shared_ptr<Session>()> sessionFactory, string ip, int port, int maxSessionCounts = 1) 
	: _serviceType(type), _sessionFactory(sessionFactory), _ip(ip), _port(port), _maxSessionCounts(maxSessionCounts)
{
	SocketUtils::SetSockAddrIn(_sockAddr, _ip, _port);
}

NetService::~NetService()
{
}

shared_ptr<Session> NetService::CreateSession()
{
	shared_ptr<Session> session = _sessionFactory();
	session->_ownerNetService = this;

	_iocpCore->Register(session);

	return session;
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

/*------------------------------------------------------------------------------------------
	Client Service
-------------------------------------------------------------------------------------------*/
ClientService::ClientService(function<shared_ptr<Session>()> sessionFactory, string ip, int port, int maxSessionCounts)
	: NetService(ServiceType::CLIENT, sessionFactory, ip, port, maxSessionCounts)
{
}
bool ClientService::Init()
{
	_iocpCore = make_shared<IocpCore>();

	// 努扼 技记 家南 目池飘
	for (int i = 0; i < _maxSessionCounts; i++) {
		shared_ptr<Session> session = CreateSession();
		session->_ownerNetService = this;
		if (session->Connect() == false) {
			cout << "ClientService::Init() Error " << endl;
			return false;
		}
	}

	cout << "[ClientService] Connected to " << _ip << ":" << _port << " OK" << endl;

	return true;
}
/*------------------------------------------------------------------------------------------
	Server Service
-------------------------------------------------------------------------------------------*/

ServerService::ServerService(function<shared_ptr<Session>()> sessionFactory, string ip, int port, int maxSessionCounts)
	: NetService(ServiceType::SERVER, sessionFactory, ip, port, maxSessionCounts)
{
}

bool ServerService::Init()
{
	_iocpCore = make_shared<IocpCore>();

	_listenSession = make_shared<ListenSession>(this);
	if (_listenSession->Init() == false)
		return false;

	cout << "[ServerService] " << _ip << ":" << _port << " Listen Init OK. now Listening..." << endl;

	return true;
}
