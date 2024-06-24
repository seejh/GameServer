#include "pch.h"
#include "RoomManager.h"

#include"Room.h"

void RoomManager::Update()
{
	for (auto r : _rooms) {
		r.second->Update();
	}
}

// �ش� �뿡 � ���� �ҷ��� ������ ���ڷ� ����
shared_ptr<Room> RoomManager::Add(int mapId)
{
	shared_ptr<Room> room = make_shared<Room>();
	room->Init(-500, 9500, -3500, 500); // minX, maxX, minY, maxY

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

shared_ptr<Room> RoomManager::Find(int roomId)
{
	auto it = _rooms.find(roomId);
	if (it != _rooms.end()) 
		return it->second;

	return nullptr;
}
