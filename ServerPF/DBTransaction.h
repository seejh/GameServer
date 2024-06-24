#pragma once


class Player;
class Room;
class DBConnection;
class DBTransaction : public JobQueue
{
public:
	static shared_ptr<DBTransaction> Instance() {
		static shared_ptr<DBTransaction> instance;
		if (instance == nullptr) 
			instance = make_shared<DBTransaction>();

		return instance;
	}

	void Init();
	void RewardPlayer(shared_ptr<Player> plyaer, RewardData reward, shared_ptr<Room> room);
public:
	DBConnection* dbConn = nullptr;
};

