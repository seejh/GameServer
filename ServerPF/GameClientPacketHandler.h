#pragma once

#include"Protocol3.pb.h"
#include"Session.h"
#include"SendBuffer.h"

// protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

enum {

	PacketCount = 300,
};


extern function<bool(shared_ptr<PacketSession>&, char*, int)> GGamePacketHandler[PacketCount];

bool Handle_INVALID(shared_ptr<PacketSession>& session, char* buffer, int len);
bool Handle_C_LOGIN(shared_ptr<PacketSession>& session, PROTOCOL::C_LOGIN fromPkt);
bool Handle_C_ENTER_ROOM(shared_ptr<PacketSession>& session, PROTOCOL::C_ENTER_ROOM fromPkt);
bool Handle_C_LEAVE_ROOM(shared_ptr<PacketSession>& session, PROTOCOL::C_LEAVE_ROOM fromPkt);
bool Handle_C_SPAWN(shared_ptr<PacketSession>& session, PROTOCOL::C_SPAWN fromPkt);
bool Handle_C_MOVE(shared_ptr<PacketSession>& session, PROTOCOL::C_MOVE fromPkt);
bool Handle_C_Skill(shared_ptr<PacketSession>& session, PROTOCOL::C_SKILL fromPkt);
bool Handle_C_CHAT(shared_ptr<PacketSession>& session, PROTOCOL::C_CHAT fromPkt);
bool Handle_C_ADDITEM(shared_ptr<PacketSession>& session, PROTOCOL::C_ADD_ITEM fromPkt);
bool Handle_C_EQUIPITEM(shared_ptr<PacketSession>& session, PROTOCOL::C_EQUIP_ITEM fromPkt);
bool Handle_C_USEITEM(shared_ptr<PacketSession>& session, PROTOCOL::C_USE_ITEM fromPkt);
bool Handle_C_CREATE_PLAYER(shared_ptr<PacketSession>& session, PROTOCOL::C_CREATE_PLAYER fromPkt);
bool Handle_C_AddQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_ADD_QUEST fromPkt);
bool Handle_C_RemoveQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_REMOVE_QUEST fromPkt);
bool Handle_C_CompleteQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_COMPLETE_QUEST fromPkt);
bool Handle_C_UpdateQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_UPDATE_QUEST fromPkt);
bool Handle_C_BotLogin(shared_ptr<PacketSession>& session, PROTOCOL::C_BOT_LOGIN fromPkt);
bool Handle_C_Test(shared_ptr<PacketSession>& session, PROTOCOL::C_TEST fromPkt);


class GameClientPacketHandler
{
public:
	static void Init() {

		for (int i = 0; i < PacketCount; i++) 
			GGamePacketHandler[i] = Handle_INVALID;

		GGamePacketHandler[PROTOCOL::MsgId::c_login] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_LOGIN>(Handle_C_LOGIN, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_enter_room] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_ENTER_ROOM>(Handle_C_ENTER_ROOM, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_leave_room] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_LEAVE_ROOM>(Handle_C_LEAVE_ROOM, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_spawn] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_SPAWN>(Handle_C_SPAWN, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_move] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_MOVE>(Handle_C_MOVE, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_skill] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_SKILL>(Handle_C_Skill, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_chat] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_CHAT>(Handle_C_CHAT, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_add_item] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_ADD_ITEM>(Handle_C_ADDITEM, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_equip_item] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_EQUIP_ITEM>(Handle_C_EQUIPITEM, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_use_item] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_USE_ITEM>(Handle_C_USEITEM, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_create_player] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_CREATE_PLAYER>(Handle_C_CREATE_PLAYER, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_add_quest] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_ADD_QUEST>(Handle_C_AddQuest, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_remove_quest] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_REMOVE_QUEST>(Handle_C_RemoveQuest, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_complete_quest] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_COMPLETE_QUEST>(Handle_C_CompleteQuest, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_update_quest] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_UPDATE_QUEST>(Handle_C_UpdateQuest, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_bot_login] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_BOT_LOGIN>(Handle_C_BotLogin, session, buffer, len);
		};
		GGamePacketHandler[PROTOCOL::MsgId::c_test] = [](shared_ptr<PacketSession>& session, char* buffer, int len) {
			return HandlePacket<PROTOCOL::C_TEST>(Handle_C_Test, session, buffer, len);
		};
	}

	static bool HandlePacket(shared_ptr<PacketSession>& session, char* buffer, int len) {
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		
		return GGamePacketHandler[header->_id](session, buffer, len);
	}

	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_LOGIN& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_login);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ENTER_ROOM& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_enter_room);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_LEAVE_ROOM& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_leave_room);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_SPAWN& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_spawn);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_DESPAWN& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_despawn);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ITEMLIST& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_itemlist);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_MOVE& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_move);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_SKILL& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_skill);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_CHAT& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_chat);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_CHANGE_HP& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_change_hp);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_DIE& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_die);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_EQUIP_ITEM& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_equip_item);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_USE_ITEM& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_use_item);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ADD_ITEM& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_add_item);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_UPDATE_ITEM& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_update_item);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_REMOVE_ITEM& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_remove_item);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_CHANGE_STAT& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_change_stat);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_CREATE_PLAYER& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_create_player);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ADD_EXP& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_add_exp);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_LEVEL_UP& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_level_up);

	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_ADD_QUEST& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_add_quest);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_QUESTLIST& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_questlist);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_REMOVE_QUEST& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_remove_quest);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_COMPLETE_QUEST& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_complete_quest);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_UPDATE_QUEST& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_update_quest);
	}
	static shared_ptr<SendBuffer> MakeSendBuffer(PROTOCOL::S_TEST& pkt) {
		return MakeSendBuffer(pkt, PROTOCOL::MsgId::s_test);
	}

private:
	template<typename PktType, typename Func>
	static bool HandlePacket(Func func, shared_ptr<PacketSession>& session, char* buffer, int len) {
		PktType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename PKT>
	static shared_ptr<SendBuffer> MakeSendBuffer(PKT& pkt, int pktId) {
		int dataSize = static_cast<int>(pkt.ByteSizeLong());
		int totalSize = dataSize + sizeof(PacketHeader);

		// 샌드버퍼
		shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(totalSize);

		// 패킷헤더
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		new(header)PacketHeader();
		header->_id = pktId;
		header->_size = totalSize;

		// 직렬화
		if (!pkt.SerializeToArray(sendBuffer->Buffer() + sizeof(PacketHeader), dataSize)) {
			cout << "packetHandler makePacket error" << endl;
			return nullptr;
		}

		return sendBuffer;
	}
};


