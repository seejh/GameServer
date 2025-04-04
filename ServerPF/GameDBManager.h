#pragma once
#include"DBDataModel.h"

class Player;
class GameRoom;
class DBConnection;
class GameSession;
class Item;
class Quest;
class GameDBManager : public JobQueue {
public:
	bool Connect(int32 connectionCounts, const WCHAR* connectionString);
	bool ConnectA(int32 connectionCounts, const CHAR* connectionString);

	DBConnection* Pop();
	void Push(DBConnection* dbConn);

	/*-------------------------------------------------------------------------------
		Room
	--------------------------------------------------------------------------------*/
	/*void InsertItem(shared_ptr<Player> player, ItemDB itemDB, void (Room::*memFunc)(shared_ptr<Player>, ItemDB));
	void UpdateItem(shared_ptr<Player> player, ItemDB itemDB, void (Room::*memFunc)(shared_ptr<Player>, ItemDB));
	void DeleteItem(shared_ptr<Player> player, ItemDB itemDB, void (Room::*memFunc)(shared_ptr<Player>, ItemDB));
	void InsertQuest(shared_ptr<Player> player, QuestDB questDB, void (Room::*memFunc)(shared_ptr<Player>, QuestDB));
	void UpdateQuest(shared_ptr<Player> player, QuestDB questDB, void (Room::*memFunc)(shared_ptr<Player>, QuestDB));
	void DeleteQuest(shared_ptr<Player> player, QuestDB questDB, void (Room::*memFunc)(shared_ptr<Player>, QuestDB));*/


	
	// Noti
	void TransactNoti_EquipItem(shared_ptr<Player> player, shared_ptr<Item> equipment);
	void TransactNoti_UpdateQuest(shared_ptr<Player> player, QuestDB questDB);

	// Transact
	// 퀘스트
	void Transact_CompleteQuest(shared_ptr<Player> player, QuestDB questDB, vector<ItemDB> itemDBs);
	void Transact_AddQuest(shared_ptr<Player> player, QuestDB questDB);
	void Transact_RemoveQuest(shared_ptr<Player> player, QuestDB questDB);

	// 아이템
	void Transact_UseItem(shared_ptr<Player> player, ItemDB itemDB);
	void Transact_AddItem(shared_ptr<Player> player, ItemDB itemDB);

public:
	DBConnection* _dbConn = nullptr;
	DBConnectionPool* _dbConnPool;

	int32 _connectionCounts;
};