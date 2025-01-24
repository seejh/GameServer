#include "pch.h"
#include "Session.h"

#include"IocpEvent.h"
#include"NetService.h"
#include"ListenSession.h"

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
	case EventType::CONNECT:
		ProcessConnect();
		break;
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

bool Session::Connect()
{
	return RegisterConnect();
}

bool Session::RegisterConnect()
{
	if (_isConnected)
		return false;

	if (_ownerNetService->_serviceType != ServiceType::CLIENT)
		return false;

	if (SocketUtils::SetReuseAddr(_socket, true) == false)
		return false;

	// 이걸 누락하면 ConnectEx()에서 에러 발생
	if (SocketUtils::BindAnyAddress(_socket, 0) == false)
		return false;

	_connectEvent.OverlappedReset();
	_connectEvent._ownerIocpObject = shared_from_this();

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = _ownerNetService->_listenSession->_serverAddr;

	if (false == SocketUtils::_connectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr),
		sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent)) {

		int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {

			cout << "Session::RegisterConnect() Error" << endl;

			_connectEvent._ownerIocpObject = nullptr;
			return false;
		}
	}

	return true;
}

void Session::ProcessConnect()
{
	_connectEvent._ownerIocpObject = nullptr;

	_isConnected.store(true);

	_ownerNetService->AddSession(static_pointer_cast<Session>(shared_from_this()));

	OnConnected();

	RegisterRecv();
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
	// RecvEvent 리셋
	_recvEvent.OverlappedReset();
	_recvEvent._ownerIocpObject.reset();

	// 연결 종료 
	if (!_isConnected.load())
		return;

	// 0 바이트 수신 에러
	if (recvLen <= 0) {
		Disconnect("RecvLen<=0");
		return;
	}

	// RecvBuffer Recv 양만큼 기록
	if (!_recvBuffer.OnWrite(recvLen)) {
		Disconnect("RecvBuffer Write overflow");
		return;
	}

	int processLen = OnRecv(_recvBuffer.BufferReadPos(), _recvBuffer.DataSize());
	if (!_recvBuffer.OnRead(processLen)) {
		Disconnect("RecvBuffer Read overflow");
		return;
	}

	// RecvBuffer 리셋
	_recvBuffer.Reset();

	// 수신 대기 다시 걸어줌
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


	// 락 걸고 큐 교환
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

	// Disconnect 한 번만 걸기위해서

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
	// len = Now Buffer TotalSize;
	
	int processLen = 0;
	
	while (true) {
		int nowData = len - processLen;

		// 헤더 사이즈 보다 적다면
		if (nowData < sizeof(PacketHeader))
			break;

		// 헤더에서 명시한 사이즈(패킷 바디 + 헤더) 보다 적다면
		PacketHeader* header = reinterpret_cast<PacketHeader*>(&buffer[processLen]);
		if (nowData < header->_size)
			break;

		// 
		OnRecvPacket(&buffer[processLen], header->_size);
		processLen += header->_size;
	}
	
	return processLen;
}

