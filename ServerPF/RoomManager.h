#pragma once

class GameRoom;

// 고칠 필요가 있음
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

	shared_ptr<GameRoom> Add(int mapId);
	bool Remove(int roomId);

	shared_ptr<GameRoom> Find(int roomId);

	// void Update();

public:
	map<int, shared_ptr<GameRoom>> _rooms;
	int _roomId = 1;
};

