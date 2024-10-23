#pragma once

#include"Session.h"

class Player;
class ClientSession : public PacketSession {

public:
	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnSend(int len) override;

	virtual void OnRecvPacket(char* buffer, int len) override;

	void SendPacket(shared_ptr<SendBuffer> sendBuffer);
	void SendPacketFlush();

public:
	void HandleLogin(PROTOCOL::C_Login fromPkt);
	void HandleCreatePlayer(string name);
	void HandleEnterRoom(PROTOCOL::C_Enter_Room fromPkt);
public:
	mutex _packetBufferLock;
	vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>> _packetBuffer;

	
	shared_ptr<Player> _player;
	int _accountDbId;
	vector<PROTOCOL::ObjectInfo> _myCharacterList;
	PROTOCOL::PlayerServerState _playerState = PROTOCOL::PlayerServerState::SERVER_STATE_NONE;
};

