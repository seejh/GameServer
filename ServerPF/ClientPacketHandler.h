#pragma once

#include"Protocol3.pb.h"


#include"SendBuffer.h"
#include"ClientSession.h"

// protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
class ClientSession;
enum {

	PacketCount = 300,
};

extern function<bool(shared_ptr<ClientSession>, char*, int)> GPacketHandler[PacketCount];

bool Handle_INVALID(shared_ptr<ClientSession> session, char* buffer, int len);
bool Handle_C_LOGIN(shared_ptr<ClientSession> session, PROTOCOL::C_Login fromPkt);
bool Handle_C_ENTER_ROOM(shared_ptr<ClientSession> session, PROTOCOL::C_Enter_Room fromPkt);
bool Handle_C_LEAVE_ROOM(shared_ptr<ClientSession> session, PROTOCOL::C_Leave_Room fromPkt);
bool Handle_C_SPAWN(shared_ptr<ClientSession> session, PROTOCOL::C_Spawn fromPkt);
bool Handle_C_MOVE(shared_ptr<ClientSession> session, PROTOCOL::C_Move fromPkt);
bool Handle_C_Skill(shared_ptr<ClientSession> session, PROTOCOL::C_Skill fromPkt);
bool Handle_C_CHAT(shared_ptr<ClientSession> session, PROTOCOL::C_Chat fromPkt);
bool Handle_C_ADDITEM(shared_ptr<ClientSession> session, PROTOCOL::C_AddItem fromPkt);
bool Handle_C_EQUIPITEM(shared_ptr<ClientSession> session, PROTOCOL::C_EquipItem fromPkt);
bool Handle_C_USEITEM(shared_ptr<ClientSession> session, PROTOCOL::C_UseItem fromPkt);
bool Handle_C_CREATE_PLAYER(shared_ptr<ClientSession> session, PROTOCOL::C_CreatePlayer fromPkt);
bool Handle_C_AddQuest(shared_ptr<ClientSession> session, PROTOCOL::C_AddQuest fromPkt);
bool Handle_C_RemoveQuest(shared_ptr<ClientSession> session, PROTOCOL::C_RemoveQuest fromPkt);
bool Handle_C_CompleteQuest(shared_ptr<ClientSession> session, PROTOCOL::C_CompleteQuest fromPkt);
bool Handle_C_UpdateQuest(shared_ptr<ClientSession> session, PROTOCOL::C_UpdateQuest fromPkt);
bool Handle_C_Test(shared_ptr<ClientSession> session, PROTOCOL::C_Test fromPkt);

class ClientPacketHandler
{
public:
	static void Init() {

		for (int i = 0; i < PacketCount; i++) 
			GPacketHandler[i] = Handle_INVALID;

		GPacketHandler[PROTOCOL::MsgId::C_LOGIN] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Login>(Handle_C_LOGIN, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_ENTER_ROOM] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Enter_Room>(Handle_C_ENTER_ROOM, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_LEAVE_ROOM] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Leave_Room>(Handle_C_LEAVE_ROOM, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_SPAWN] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Spawn>(Handle_C_SPAWN, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_MOVE] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Move>(Handle_C_MOVE, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_SKILL] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Skill>(Handle_C_Skill, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_CHAT] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Chat>(Handle_C_CHAT, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_ADD_ITEM] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_AddItem>(Handle_C_ADDITEM, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_EQUIPITEM] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_EquipItem>(Handle_C_EQUIPITEM, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_USEITEM] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_UseItem>(Handle_C_USEITEM, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_CREATE_PLAYER] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_CreatePlayer>(Handle_C_CREATE_PLAYER, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_ADD_QUEST] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_AddQuest>(Handle_C_AddQuest, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_REMOVE_QUEST] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_RemoveQuest>(Handle_C_RemoveQuest, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_COMPLETE_QUEST] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_CompleteQuest>(Handle_C_CompleteQuest, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_UPDATE_QUEST] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_UpdateQuest>(Handle_C_UpdateQuest, session, buffer, len);
		};
		GPacketHandler[PROTOCOL::MsgId::C_TEST] = [](shared_ptr<ClientSession> session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_Test>(Handle_C_Test, session, buffer, len);
		};
	}

	static bool HandlePacket(shared_ptr<ClientSession> session, char* buffer, int len) {
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->_id](session, buffer, len);
	}

	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Login& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_LOGIN);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Enter_Room& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_ENTER_ROOM);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Leave_Room& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_LEAVE_ROOM);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Spawn& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_SPAWN);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_DeSpawn& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_DESPAWN);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ItemList& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_ITEM_LIST);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Move& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_MOVE);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Skill& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_SKILL);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Chat& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_CHAT);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ChangeHp& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_CHANGE_HP);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Die& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_DIE);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_EquipItem& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_EQUIPITEM);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_UseItem& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_USEITEM);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_AddItem& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_ADD_ITEM);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_UpdateItem& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_UPDATE_ITEM);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_RemoveItem& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_REMOVE_ITEM);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ChangeStat& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_CHANGE_STAT);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_CreatePlayer& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_CREATE_PLAYER);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_AddExp& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_ADD_EXP);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_LevelUp& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_LEVEL_UP);

	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_AddQuest& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_ADD_QUEST);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_QuestList& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_QUEST_LIST);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_RemoveQuest& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_REMOVE_QUEST);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_CompleteQuest& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_COMPLETE_QUEST);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_UpdateQuest& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_UPDATE_QUEST);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_Test& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::S_TEST);
	}

private:
	template<typename PktType, typename Func>
	static bool HandlePacket(Func func, shared_ptr<ClientSession> session, char* buffer, int len) {
		PktType pkt;
		pkt.ParseFromArray(buffer + sizeof(PacketHeader), len);

		return func(session, pkt);
	}

	template<typename PKT>
	static shared_ptr<SendBuffer> MakeSendBuffer(PKT& pkt, int pktId) {
		int packetSize = static_cast<int>(pkt.ByteSizeLong());
		int totalSize = packetSize + sizeof(PacketHeader);

		shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(totalSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		new(header)PacketHeader();
		header->_id = pktId;
		header->_size = packetSize;

		if (!pkt.SerializeToArray(sendBuffer->Buffer() + sizeof(PacketHeader), header->_size)) {
			cout << "packetHandler makePacket error" << endl;
			return nullptr;
		}

		return sendBuffer;
	}
};


