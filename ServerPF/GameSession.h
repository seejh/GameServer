#pragma once

#include"Session.h"

class Player;
class GameSession : public PacketSession {

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnSend(int len) override;

	virtual void OnRecvPacket(char* buffer, int len) override;

	// 보낼 패킷 예약만
	void SendPacket(shared_ptr<SendBuffer> sendBuffer);
	// 쌓인 패킷 플러시
	void SendPacketFlush();

public:
	void HandleLogin(PROTOCOL::C_LOGIN fromPkt);
	void HandleCreatePlayer(string name);
	void HandleEnterRoom(PROTOCOL::C_ENTER_ROOM fromPkt);

	void TestBotLogin();
public:
	int32 _sessionId;

	mutex _mutex2;
	vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>> _packetBuffer;

	shared_ptr<Player> _player;
	int _accountDbId;
	vector<PROTOCOL::ObjectInfo> _myCharacterList;
	PROTOCOL::PlayerServerState _playerState = PROTOCOL::PlayerServerState::SERVER_STATE_NONE;
};

