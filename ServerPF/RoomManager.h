#pragma once

class Room;

class RoomManager : public JobQueue
{
public:
	static shared_ptr<RoomManager> Instance() {
		static shared_ptr<RoomManager> instance;
		if (instance == nullptr)
			instance = make_shared<RoomManager>();

		return instance;
	}

	bool Init();

	shared_ptr<Room> Add(int mapId);
	bool Remove(int roomId);

	shared_ptr<Room> Find(int roomId);

	// void Update();

public:
	map<int, shared_ptr<Room>> _rooms;
	int _roomId = 1;
};

