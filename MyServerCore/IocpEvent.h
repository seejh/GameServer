#pragma once

class IocpObject;
class Session;

class SendBuffer;


enum EventType { ACCEPT, CONNECT, DISCONNECT, SEND, RECV, };

// 이벤트는 스택에서 관리될 것이며, 스택 내부에서 사용되는 스마트 포인터 멤버들은 사용이 끝나면 일일이 할당 해제하는 방식이므로
// 여기서 소멸자를 만들어서 관리할 필요가 없다.
// 주의 WSAOVERLAPPED를 할당 받는 식으로 만들었는데, virtual 써서 오프셋 밀림
class IocpEvent : public WSAOVERLAPPED
{
public:

	IocpEvent(EventType type) : _type(type) { OverlappedReset(); }

	void OverlappedReset() {

		hEvent = 0;
		Internal = 0;
		InternalHigh = 0;
		Offset = 0;
		OffsetHigh = 0;
	}
public:
	EventType _type;
	shared_ptr<IocpObject> _ownerIocpObject;
};

class AcceptEvent : public IocpEvent {
public:
	AcceptEvent() : IocpEvent(EventType::ACCEPT) {}
public:
	shared_ptr<Session> _connectedSession;
};

class ConnectEvent : public IocpEvent {
public:
	ConnectEvent() : IocpEvent(EventType::CONNECT) {}
};

class DisconnectEvent : public IocpEvent {
public:
	DisconnectEvent() : IocpEvent(EventType::DISCONNECT) {}
};

class SendEvent : public IocpEvent {
public:
	SendEvent() : IocpEvent(EventType::SEND) {}

public:
	vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>> _sendEventBuffer;

};

class RecvEvent : public IocpEvent {
public:
	RecvEvent() : IocpEvent(EventType::RECV) {}
};