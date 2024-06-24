#pragma once

#include"IocpCore.h"
#include"IocpEvent.h"

class NetService;

class ListenSession : public IocpObject
{
public:
	ListenSession(NetService* netService);
	~ListenSession();

	// override
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(IocpEvent* iocpEvent, int len) override;

	bool Init();

	bool RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

	NetService* OwnerNetService();
public:
	SOCKET _listenSocket;
	SOCKADDR_IN _serverAddr;

	NetService* _ownerNetService;

	AcceptEvent* _acceptEvents;
	int _acceptCount = 5;
};

