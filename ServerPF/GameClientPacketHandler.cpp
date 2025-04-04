#include "pch.h"
#include "GameClientPacketHandler.h"

#include"GameRoom.h"
#include"RoomManager.h"
#include"GameObject.h"
#include"GameSession.h"

#include"DBBind.h"
#include"DBConnectionPool.h"

#include"DBSynchronizer.h"
#include"GenProcedures.h"
#include"DBDataModel.h"
#include"JwtHandler.h"
#include"QuestManager.h"
#include"GameSessionManager.h"

function<bool(shared_ptr<PacketSession>&, char*, int)> GGamePacketHandler[PacketCount];

bool Handle_INVALID(shared_ptr<PacketSession>& session, char* buffer, int len)
{

	cout << endl << "Handle INVALID PACKET" << endl;
	return false;
}

bool Handle_C_LOGIN(shared_ptr<PacketSession>& session, PROTOCOL::C_LOGIN fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	clientSession->HandleLogin(fromPkt);

	return true;
}


bool Handle_C_ENTER_ROOM(shared_ptr<PacketSession>& session, PROTOCOL::C_ENTER_ROOM fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	clientSession->HandleEnterRoom(fromPkt);
	
	return true;
}


bool Handle_C_LEAVE_ROOM(shared_ptr<PacketSession>& session, PROTOCOL::C_LEAVE_ROOM fromPkt)
{
	return false;
}

bool Handle_C_SPAWN(shared_ptr<PacketSession>& session, PROTOCOL::C_SPAWN fromPkt)
{
	
	return false;
}

bool Handle_C_MOVE(shared_ptr<PacketSession>& session, PROTOCOL::C_MOVE fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr) {
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::ActorMove, static_pointer_cast<GameObject>(clientSession->_player), fromPkt.object().pos().location());
	}

	return true;
}

bool Handle_C_Skill(shared_ptr<PacketSession>& session, PROTOCOL::C_SKILL fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	if(clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::ActorSkill, static_pointer_cast<GameObject>(clientSession->_player), fromPkt);

	return true;
}


bool Handle_C_CHAT(shared_ptr<PacketSession>& session, PROTOCOL::C_CHAT fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::PlayerChat, clientSession->_player, fromPkt);

	return true;
}

bool Handle_C_ADDITEM(shared_ptr<PacketSession>& session, PROTOCOL::C_ADD_ITEM fromPkt)
{
	//if(session->_player != nullptr && session->_player->_ownerRoom != nullptr)
	//	session->_player->_ownerRoom->DoAsync(&Room::HandleAddItem, session->_player, fromPkt);

	return true;
}

bool Handle_C_EQUIPITEM(shared_ptr<PacketSession>& session, PROTOCOL::C_EQUIP_ITEM fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	// 변경
	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::HandleEquipItem, clientSession->_player, fromPkt);
	
	return true;
}

bool Handle_C_USEITEM(shared_ptr<PacketSession>& session, PROTOCOL::C_USE_ITEM fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	// 변경
	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::HandleUseItem, clientSession->_player, fromPkt);

	return true;
}

bool Handle_C_CREATE_PLAYER(shared_ptr<PacketSession>& session, PROTOCOL::C_CREATE_PLAYER fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	clientSession->HandleCreatePlayer(fromPkt.name());

	return true;
}

bool Handle_C_AddQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_ADD_QUEST fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::HandleAddQuest, clientSession->_player, fromPkt);

	return true;
}

bool Handle_C_RemoveQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_REMOVE_QUEST fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::HandleRemoveQuest, clientSession->_player, fromPkt);

	return true;
}

bool Handle_C_CompleteQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_COMPLETE_QUEST fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::HandleCompleteQuest, clientSession->_player, fromPkt);

	return true;
}

bool Handle_C_UpdateQuest(shared_ptr<PacketSession>& session, PROTOCOL::C_UPDATE_QUEST fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	if (clientSession->_player != nullptr && clientSession->_player->_ownerRoom != nullptr)
		clientSession->_player->_ownerRoom->DoAsync(&GameRoom::HandleUpdateQuest, clientSession->_player, fromPkt);

	return true;
}

bool Handle_C_BotLogin(shared_ptr<PacketSession>& session, PROTOCOL::C_BOT_LOGIN fromPkt)
{
	shared_ptr<GameSession> clientSession = static_pointer_cast<GameSession>(session);

	clientSession->TestBotLogin();

	return true;
}

bool Handle_C_Test(shared_ptr<PacketSession>& session, PROTOCOL::C_TEST fromPkt)
{
	cout << "Handle_C_TEST" << endl;

	return true;
}

