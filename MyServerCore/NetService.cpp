#include "pch.h"
#include "NetService.h"
#include"ListenSession.h"

/*------------------------------------------------------------------------------------------
	Service
-------------------------------------------------------------------------------------------*/

NetService::NetService(ServiceType type, function<shared_ptr<Session>()> sessionFactory, wstring ip, int port, int maxSessionCounts = 1) 
	: _serviceType(type), _sessionFactory(sessionFactory), _ip(ip), _port(port), _maxSessionCounts(maxSessionCounts)
{
}

NetService::~NetService()
{
}

bool NetService::Init()
{
	_iocpCore = make_shared<IocpCore>();

	// wstring ->string
	int needSize = WideCharToMultiByte(CP_UTF8, 0, &_ip[0], (int)_ip.size(), NULL, 0, NULL, NULL);
	string strIp(needSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, &_ip[0], (int)_ip.size(), &strIp[0], needSize, NULL, NULL);

	_listenSession = make_shared<ListenSession>(this);
	if (_listenSession->Init(strIp, _port) == false)
		return false;

	cout << strIp << ":" << _port << " Listen Init OK. Now Listening..." << endl;

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

/*------------------------------------------------------------------------------------------
	Client Service
-------------------------------------------------------------------------------------------*/
ClientService::ClientService(function<shared_ptr<Session>()> sessionFactory, wstring ip, int port, int maxSessionCounts)
	: NetService(ServiceType::CLIENT, sessionFactory, ip, port, maxSessionCounts)
{
}
bool ClientService::Init()
{
	_iocpCore = make_shared<IocpCore>();

	// wstring ->string
	int needSize = WideCharToMultiByte(CP_UTF8, 0, &_ip[0], (int)_ip.size(), NULL, 0, NULL, NULL);
	string strIp(needSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, &_ip[0], (int)_ip.size(), &strIp[0], needSize, NULL, NULL);

	// 努扼 技记 家南 目池飘
	for (int i = 0; i < _maxSessionCounts; i++) {
		shared_ptr<Session> session = CreateSession();
		if (session->Connect() == false) {
			cout << "ClientService::Init() Error " << endl;
			return false;
		}
	}

	cout << "[ClientService] Connected to" << strIp << ":" << _port << " OK" << endl;

	return true;
}
/*------------------------------------------------------------------------------------------
	Server Service
-------------------------------------------------------------------------------------------*/

ServerService::ServerService(function<shared_ptr<Session>()> sessionFactory, wstring ip, int port, int maxSessionCounts)
	: NetService(ServiceType::SERVER, sessionFactory, ip, port, maxSessionCounts)
{
}

bool ServerService::Init()
{
	_iocpCore = make_shared<IocpCore>();

	// wstring ->string
	int needSize = WideCharToMultiByte(CP_UTF8, 0, &_ip[0], (int)_ip.size(), NULL, 0, NULL, NULL);
	string strIp(needSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, &_ip[0], (int)_ip.size(), &strIp[0], needSize, NULL, NULL);

	_listenSession = make_shared<ListenSession>(this);
	if (_listenSession->Init(strIp, _port) == false)
		return false;

	cout << "[ServerService] " << strIp << ":" << _port << " Listen Init OK. now Listening..." << endl;

	return true;
}
