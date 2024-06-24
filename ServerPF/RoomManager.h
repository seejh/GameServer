#pragma once

class Room;

class RoomManager : public JobQueue
{
public:
	static RoomManager* Instance() {
		static RoomManager instance;
		return &instance;
	}

	void Update();

	shared_ptr<Room> Add(int mapId);
	bool Remove(int roomId);
	shared_ptr<Room> Find(int roomId);

public:
	map<int, shared_ptr<Room>> _rooms;
	int _roomId = 1;
};

