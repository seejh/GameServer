#include "pch.h"
#include "ClientPacketHandler.h"

#include"Room.h"
#include"RoomManager.h"
#include"GameObject.h"

#include"ClientSession.h"

#include"DBBind.h"
#include"DBConnectionPool.h"

#include"DBSynchronizer.h"
#include"GenProcedures.h"
#include"DBDataModel.h"


function<bool(shared_ptr<ClientSession>, char*, int)> GPacketHandler[PacketCount];

bool Handle_INVALID(shared_ptr<ClientSession> session, char* buffer, int len)
{

	cout << endl << "Handle INVALID PACKET" << endl;
	return false;
}

bool Handle_C_LOGIN(shared_ptr<ClientSession> session, PROTOCOL::C_Login fromPkt)
{
	session->HandleLogin(fromPkt);


	return true;
}


bool Handle_C_ENTER_ROOM(shared_ptr<ClientSession> session, PROTOCOL::C_Enter_Room fromPkt)
{
	session->HandleEnterRoom(fromPkt);

	
	return true;
}


bool Handle_C_LEAVE_ROOM(shared_ptr<ClientSession> session, PROTOCOL::C_Leave_Room fromPkt)
{
	return false;
}

bool Handle_C_SPAWN(shared_ptr<ClientSession> session, PROTOCOL::C_Spawn fromPkt)
{
	
	return false;
}

bool Handle_C_MOVE(shared_ptr<ClientSession> session, PROTOCOL::C_Move fromPkt)
{
	// cout << "PLAYER:" << session->_player->info.objectid() << ", Move" << endl;

	if (session->_player->_ownerRoom != nullptr)
		session->_player->_ownerRoom->DoAsync(&Room::ActorMove, static_pointer_cast<GameObject>(session->_player), fromPkt.object());

	return true;
}

bool Handle_C_Skill(shared_ptr<ClientSession> session, PROTOCOL::C_Skill fromPkt)
{
	if (session->_player->_ownerRoom != nullptr)
		session->_player->_ownerRoom->DoAsync(&Room::ActorSkill, static_pointer_cast<GameObject>(session->_player), fromPkt);

	
	return true;
}


bool Handle_C_CHAT(shared_ptr<ClientSession> session, PROTOCOL::C_Chat fromPkt)
{
	session->_player->_ownerRoom->DoAsync(&Room::PlayerChat, session->_player, fromPkt);

	return true;
}

bool Handle_C_USEITEM(shared_ptr<ClientSession> session, PROTOCOL::C_UseItem fromPkt)
{
	if (session->_player->_ownerRoom != nullptr)
		session->_player->_ownerRoom->DoAsync(&Room::ActorUseItem, static_pointer_cast<GameObject>(session->_player), fromPkt);
	
	return true;
}

bool Handle_C_CREATE_PLAYER(shared_ptr<ClientSession> session, PROTOCOL::C_CreatePlayer fromPkt)
{
	session->HandleCreatePlayer(fromPkt.name());


	return true;
}

