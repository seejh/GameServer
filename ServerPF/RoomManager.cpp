#include "pch.h"
#include "RoomManager.h"

#include"GameRoom.h"



bool RoomManager::Init()
{
	if (Add(1) == nullptr)
		return false;

	return true;
}

// 해당 룸에 어떤 맵을 불러올 것인지 인자로 받음
shared_ptr<GameRoom> RoomManager::Add(int mapId)
{
	shared_ptr<GameRoom> room = make_shared<GameRoom>();
	room->Init();

	_rooms[_roomId] = room;
	_roomId++;

	return room;
}

bool RoomManager::Remove(int roomId)
{
	if (0 != _rooms.erase(roomId))
		return true;

	return false;
}

shared_ptr<GameRoom> RoomManager::Find(int roomId)
{
	auto it = _rooms.find(roomId);
	if (it != _rooms.end()) 
		return it->second;

	return nullptr;
}

//void RoomManager::Update()
//{
//	for (auto r : _rooms) 
//		r.second->Update();
//}