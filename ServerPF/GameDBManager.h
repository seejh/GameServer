#pragma once
#include"DBDataModel.h"

class Player;
class Room;
class DBConnection;
class ClientSession;
//class GameDBManager
//{
//public:
//	bool Connect(int32 connectionCounts, const WCHAR* connectionString);
//	// DBConnection* GetConn();
//	DBConnection* Pop();
//	void Push(DBConnection* dbConn);
//
//public:
//	std::mutex _m;
//
//	int32 _connectionCounts;
//	DBConnection* _dbConn = nullptr;
//};

class GameDBManager : public JobQueue {
public:
	bool Connect(int32 connectionCounts, const WCHAR* connectionString);

	DBConnection* Pop();
	void Push(DBConnection* dbConn);

	/*-------------------------------------------------------------------------------
		DB Task (Room)
	--------------------------------------------------------------------------------*/

	// 아이템
	void InsertItem(shared_ptr<Player> player, ItemDB itemDB, void (Room::*memFunc)(shared_ptr<Player>, ItemDB));
	void UpdateItem(shared_ptr<Player> player, ItemDB itemDB, void (Room::*memFunc)(shared_ptr<Player>, ItemDB));
	void DeleteItem(shared_ptr<Player> player, ItemDB itemDB, void (Room::*memFunc)(shared_ptr<Player>, ItemDB));

	// 퀘스트
	void InsertQuest(shared_ptr<Player> player, QuestDB questDB, void (Room::*memFunc)(shared_ptr<Player>, QuestDB));
	void UpdateQuest(shared_ptr<Player> player, QuestDB questDB, void (Room::*memFunc)(shared_ptr<Player>, QuestDB));
	void DeleteQuest(shared_ptr<Player> player, QuestDB questDB, void (Room::*memFunc)(shared_ptr<Player>, QuestDB));

public:
	int32 _connectionCounts;
	DBConnection* _dbConn = nullptr;
};