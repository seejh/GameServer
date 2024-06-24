#include "pch.h"
#include "Session.h"

#include"IocpEvent.h"
#include"NetService.h"


/*--------------------------------------------------
	Session
---------------------------------------------------*/


Session::Session()
{
}

Session::~Session()
{
	cout << "~Session()" << endl;
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int len)
{
	switch (iocpEvent->_type) {
	case EventType::SEND:
		ProcessSend(len);
		break;
	case EventType::RECV:
		ProcessRecv(len);
		break;
	case EventType::DISCONNECT:
		ProcessDisconnect();
		break;
	default:
		break;
	}
}

void Session::RegisterRecv()
{
	if (!_isConnected.load()) {
		return;
	}

	

	_recvEvent._ownerIocpObject = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = _recvBuffer.BufferWritePos();
	wsaBuf.len = _recvBuffer.FreeSize();
	DWORD dwBytes;
	DWORD flag = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &dwBytes, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&_recvEvent), 0)) {
		if (WSAGetLastError() != WSA_IO_PENDING) {

			SocketUtils::HandleError("RegisterRecv");
			_recvEvent.OverlappedReset();

			_recvEvent._ownerIocpObject.reset();
		}
	}
}

void Session::ProcessRecv(int recvLen)
{

	// RecvEvent ����
	_recvEvent.OverlappedReset();
	_recvEvent._ownerIocpObject.reset();

	// ���� ���� 
	if (!_isConnected.load())
		return;

	// 0 ����Ʈ ���� ����
	if (recvLen <= 0) {
		Disconnect("RecvLen<=0");
		return;
	}

	// RecvBuffer Recv �縸ŭ ���
	if (!_recvBuffer.OnWrite(recvLen)) {
		Disconnect("RecvBuffer Write overflow");
		return;
	}

	int processLen = OnRecv(_recvBuffer.BufferReadPos(), _recvBuffer.DataSize());
	if (!_recvBuffer.OnRead(processLen)) {
		Disconnect("RecvBuffer Read overflow");
		return;
	}

	// RecvBuffer ����
	_recvBuffer.Reset();

	// ���� ��� �ٽ� �ɾ���
	RegisterRecv();
}

void Session::Send(vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>>&& sendBufferArray)
{
	if (!_isConnected.load())
		return;

	bool sendRegistered = false;

	{
		lock_guard<mutex> lock(_mutex);
		
		for (shared_ptr<SendBuffer> s : sendBufferArray) 
			_sendBuffer.push_back(s);
		
		sendRegistered = _sendRegistered.exchange(true);
	}

	if (!sendRegistered)
		RegisterSend();
}

void Session::Send(shared_ptr<SendBuffer> sendBuffer)
{
	if (!_isConnected.load()) 
		return;

	bool sendRegistered = false;
	
	{
		lock_guard<mutex> lock(_mutex);

		_sendBuffer.push_back(sendBuffer);
		
		sendRegistered = _sendRegistered.exchange(true);
	}

	if (!sendRegistered)
		RegisterSend();

}

void Session::RegisterSend()
{
	if (!_isConnected.load())
		return;


	// �� �ɰ� ť ��ȯ
	{
		lock_guard<mutex> lock(_mutex);

		_sendEvent._sendEventBuffer.swap(_sendBuffer);
	}

	_sendEvent._ownerIocpObject = shared_from_this();

	// Scatter-Gather()
	vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent._sendEventBuffer.size());
	for (shared_ptr<SendBuffer> s : _sendEvent._sendEventBuffer) {
		WSABUF wsaBuf;
		wsaBuf.buf = s->Buffer();
		wsaBuf.len = s->Size();

		wsaBufs.push_back(wsaBuf);
	}

	DWORD bytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & bytes, 0, &_sendEvent, nullptr)) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			_sendEvent.OverlappedReset();
			_sendEvent._ownerIocpObject.reset();
			_sendEvent._sendEventBuffer.clear();

		}
	}
}

void Session::ProcessSend(int sendLen)
{

	// _sendEvent reset
	_sendEvent.OverlappedReset();
	_sendEvent._ownerIocpObject.reset();

	//
	if (!_isConnected.load())
		return;

	//
	if (sendLen <= 0) {
		Disconnect("SendLen<=0");
		return;
	}

	//
	_sendEvent._sendEventBuffer.clear();

	OnSend(sendLen);

	{
		lock_guard<mutex> lock(_mutex);
		if (_sendBuffer.empty())
			_sendRegistered.store(false);
		else
			RegisterSend();
	}	
}

void Session::Disconnect(const char* cause)
{
	//cout << "Disconnect() : " << cause << endl;

	// Disconnect �� ���� �ɱ����ؼ�

	if (_isConnected.exchange(false) == false)
		return;

	RegisterDisconnect();
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent._ownerIocpObject = shared_from_this();


	if (SocketUtils::_disconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0)) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			_disconnectEvent.OverlappedReset();

			_disconnectEvent._ownerIocpObject.reset();
			return false;
		}
	}

	return true;
}

void Session::ProcessDisconnect()
{
	_ownerNetService->RemoveSession(static_pointer_cast<Session>(shared_from_this()));

	_disconnectEvent.OverlappedReset();

	_disconnectEvent._ownerIocpObject.reset();

	OnDisconnected();
}


/*--------------------------------------------------
	PacketSession
---------------------------------------------------*/

PacketSession::~PacketSession()
{

}

int PacketSession::OnRecv(char* buffer, int len)
{
	int processLen = 0;
	
	while (true) {
		
		int nowData = len - processLen;

		// ��� ������ ���� ���ٸ�
		if (nowData < sizeof(PacketHeader))
			break;

		// ������� ����� ������(��Ŷ �ٵ� + ���) ���� ���ٸ�
		PacketHeader* header = reinterpret_cast<PacketHeader*>(&buffer[processLen]);
		if (nowData < header->_size + sizeof(PacketHeader))
			break;

		// 
		OnRecvPacket(&buffer[processLen], header->_size);
		processLen += header->_size + sizeof(PacketHeader);
	}
	
	return processLen;
}

