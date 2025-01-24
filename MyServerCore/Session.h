#pragma once

#include"IocpCore.h"
#include"IocpEvent.h"
#include"RecvBuffer.h"
#include"SendBuffer.h"



/*--------------------------------------------------
	Session
---------------------------------------------------*/
class NetService;

class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

	// for hooking
	virtual void OnConnected() {}
	virtual void OnDisconnected() {}
	virtual void OnSend(int len) {}

	virtual int OnRecv(char* buffer, int len) { cout << "OnRecv()" << endl; return len; }


	// override
	virtual HANDLE GetHandle();
	virtual void Dispatch(IocpEvent* iocpEvent, int len);

	// Connect
	bool Connect();
	bool RegisterConnect();
	void ProcessConnect();

	// Recv
	void RegisterRecv();
	void ProcessRecv(int recvLen);

	// Send
	void Send(vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>>&& sendBufferArray);
	void Send(shared_ptr<SendBuffer> sendBuffer);
	void RegisterSend();
	void ProcessSend(int sendLen);

	// Disconnect
	void Disconnect(const char* cause);
	bool RegisterDisconnect();
	void ProcessDisconnect();

public:
	SOCKET _socket;
	NetService* _ownerNetService;

	SendEvent _sendEvent;
	RecvEvent _recvEvent;
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	
	RecvBuffer _recvBuffer;

	vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>> _sendBuffer;
	atomic<bool> _sendRegistered;

	atomic<bool> _isConnected;

	mutex _mutex;
};


/*--------------------------------------------------
	PacketSession
---------------------------------------------------*/
struct PacketHeader {
	uint16 _size;
	uint16 _id;
};

class PacketSession : public Session {
public:
	virtual ~PacketSession();
protected:
	virtual int OnRecv(char* buffer, int len) sealed;
	virtual void OnRecvPacket(char* buffer, int32 len) abstract;
};

