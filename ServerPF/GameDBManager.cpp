#include "pch.h"
#include "GameDBManager.h"

#include"GameObject.h"
#include"GameRoom.h"
#include"DataManager.h"
#include"DBConnection.h"
#include"DBConnectionPool.h"
#include"DBDataModel.h"
#include"GenProcedures.h"
#include"Item.h"
#include"Quest.h"
#include"Inventory.h"
#include"GameSessionManager.h"


bool GameDBManager::Connect(int32 connectionCounts, const WCHAR* connectionString)
{
	if (_dbConnPool == nullptr)
		_dbConnPool = new DBConnectionPool();

	_connectionCounts = connectionCounts + 1;

	/*if (GDBConnectionPool->Connect(connectionCounts, connectionString) == false)
		return false;*/
	if (_dbConnPool->Connect(_connectionCounts, connectionString) == false) {
		cout << "[GameDBManager] Connect Error : Connect Failed" << endl;
		return false;
	}

	_dbConn = Pop();
	if (_dbConn == nullptr) {
		cout << "[GameDBManager] Connect Error : Nullptr Connection" << endl;
		return false;
	}
		
	cout << "[GameDBManager] Connect OK" << endl;

	return true;
}

bool GameDBManager::ConnectA(int32 connectionCounts, const CHAR* connectionString)
{
	if (_dbConnPool == nullptr)
		_dbConnPool = new DBConnectionPool();

	_connectionCounts = connectionCounts + 1;

	if (_dbConnPool->ConnectA(_connectionCounts, connectionString) == false) {
		cout << "[GameDBManager] Connect Error : Connect Failed" << endl;
		return false;
	}

	_dbConn = Pop();
	if (_dbConn == nullptr) {
		cout << "[GameDBManager] Connect Error : Nullptr Connection" << endl;
		return false;
	}

	cout << "[GameDBManager] Connect OK" << endl;

	return true;
}

DBConnection* GameDBManager::Pop()
{
	//GDBConnectionPool->Pop();
	return _dbConnPool->Pop();
}

void GameDBManager::Push(DBConnection* dbConn)
{
	// GDBConnectionPool->Push(dbConn);
	_dbConnPool->Push(dbConn);
}


/*-------------------------------------------------------------------------------------
	
--------------------------------------------------------------------------------------*/



void GameDBManager::TransactNoti_EquipItem(shared_ptr<Player> player, shared_ptr<Item> equipment)
{
	// Room Thread에서 수행

	ItemDB itemDB;
	itemDB.ItemDbId = equipment->_itemInfo.itemdbid();
	itemDB.PlayerDbId = equipment->_itemInfo.playerdbid();
	itemDB.TemplateId = equipment->_itemInfo.templateid();
	itemDB.Count = equipment->_itemInfo.count();
	itemDB.Slot = equipment->_itemInfo.slot();
	itemDB.Equipped = equipment->_itemInfo.equipped();

	// GameDBManager Thread에서 수행
	DoAsync(
		[this, itemDB]() mutable {
			SP::UpdateItem updateItem(*_dbConn);
			updateItem.In_ItemDbId(itemDB.ItemDbId);
			updateItem.In_PlayerDbId(itemDB.PlayerDbId);
			updateItem.In_TemplateId(itemDB.TemplateId);
			updateItem.In_Count(itemDB.Count);
			updateItem.In_Slot(itemDB.Slot);
			updateItem.In_Equipped(itemDB.Equipped);
			if (updateItem.Execute()) {

			}
			else {

			}
		}
	);
}

void GameDBManager::TransactNoti_UpdateQuest(shared_ptr<Player> player, QuestDB questDB)
{
	SP::UpdateQuest updateQuest(*_dbConn);
	updateQuest.In_QuestDbId(questDB.QuestDbId);
	updateQuest.In_PlayerDbId(questDB.PlayerDbId);
	updateQuest.In_TemplateId(questDB.TemplateId);
	updateQuest.In_Progress(questDB.Progress);
	updateQuest.In_Completed(questDB.Completed);
	if (updateQuest.Execute()) {
		questDB.dbState = DB_STATE::UPDATE_SUCCESS;
	}
	else {
		questDB.dbState = DB_STATE::UPDATE_FAILED;
	}
}

void GameDBManager::Transact_CompleteQuest(shared_ptr<Player> player, QuestDB questDB, vector<ItemDB> itemDBs)
{
	// 오토 커밋 비활성화
	_dbConn->SetAutoCommit(false);

	bool result = true;
	string resultLog = "";

	// 아이템 지급
	for (ItemDB itemDB : itemDBs) {
		wstring query;
		
		// 인서트
		if (itemDB.dbState == DB_STATE::INSERT_NEED) {
			DBBind<5, 1> dbBind(*_dbConn, 
				L"\
				SET NOCOUNT ON;\
				INSERT INTO [dbo].[Item]([TemplateId], [Count], [Slot], [Equipped], [PlayerDbId]) VALUES(?,?,?,?,?);\
				SELECT @@IDENTITY;\
				");
			dbBind.BindParam(0, itemDB.TemplateId);
			dbBind.BindParam(1, itemDB.Count);
			dbBind.BindParam(2, itemDB.Slot);
			dbBind.BindParam(3, itemDB.Equipped);
			dbBind.BindParam(4, itemDB.PlayerDbId);
			dbBind.BindCol(0, OUT itemDB.ItemDbId);
			if (dbBind.Execute()) {
				if (dbBind.Fetch()) {
					// 로그
					resultLog.append("ItemDbId : %d", itemDB.ItemDbId);
				}
				else {
					result = false;
					break;
				}
			}
			else {
				result = false;
				break;
			}
		}
		// 업데이트
		else {
			DBBind<5, 0> dbBind(*_dbConn,
				L"UPDATE [dbo].[Item] SET TemplateId = ?, Count = ?, Slot = ?, Equipped = ?, PlayerDbId = ? WHERE ItemDbId = ?");
			dbBind.BindParam(0, itemDB.TemplateId);
			dbBind.BindParam(1, itemDB.Count);
			dbBind.BindParam(2, itemDB.Slot);
			dbBind.BindParam(3, itemDB.Equipped);
			dbBind.BindParam(4, itemDB.PlayerDbId);
			dbBind.BindParam(5, itemDB.ItemDbId);
			if (dbBind.Execute()) {

			}
			else {
				result = false;
				break;
			}
		}
	}


	// 퀘스트 업데이트
	if (result == true) {
		DBBind<5, 0> dbBind(*_dbConn, L"UPDATE [dbo].[Quest] SET TemplateId = ?, Progress = ?, Completed = ?, PlayerDbId = ? WHERE QuestDbId = ?");
		dbBind.BindParam(0, questDB.TemplateId);
		dbBind.BindParam(1, questDB.Progress);
		dbBind.BindParam(2, questDB.Completed);
		dbBind.BindParam(3, questDB.PlayerDbId);
		dbBind.BindParam(4, questDB.QuestDbId);
		if (dbBind.Execute()) {

		}
		else {
			result = false;
		}
	}

	// 커밋 or 롤백
	if (result == true) {
		_dbConn->Commit();

		cout << "[DB] COMPLETE QUEST - SUCCEED, COMMIT" << endl;

		// 요청한 곳으로 
		if (player->_ownerRoom)
			player->_ownerRoom->DoAsync(&GameRoom::DB_Response_CompleteQuest, player, questDB, itemDBs);
	}
	else {
		cout << "[DB] COMPLETE QUEST - FAILED, ROLLBACK" << endl;

		_dbConn->Rollback();
	}

	// 오토 커밋 활성화
	_dbConn->SetAutoCommit(true);
}

void GameDBManager::Transact_AddQuest(shared_ptr<Player> player, QuestDB questDB)
{
	int64 identity;
	SP::InsertQuestAndSelectIdentity insertQuest(*_dbConn);
	insertQuest.In_PlayerDbId(questDB.PlayerDbId);
	insertQuest.In_TemplateId(questDB.TemplateId);
	insertQuest.In_Progress(questDB.Progress);
	insertQuest.In_Completed(questDB.Completed);
	insertQuest.Out_Identity(identity);
	if (insertQuest.Execute()) {
		if (insertQuest.Fetch()) {
			questDB.QuestDbId = identity;
			questDB.dbState = DB_STATE::INSERT_SUCCESS;
		}
	}
	else {
		questDB.dbState = DB_STATE::INSERT_FAILED;
	}

	if (player->_ownerRoom)
		player->_ownerRoom->DoAsync(&GameRoom::DB_Response_AddQuest, player, questDB);
}

void GameDBManager::Transact_RemoveQuest(shared_ptr<Player> player, QuestDB questDB)
{
	SP::DeleteQuest deleteQuest(*_dbConn);
	deleteQuest.In_QuestDbId(questDB.QuestDbId);
	if (deleteQuest.Execute()) {
		questDB.dbState = DB_STATE::DELETE_SUCCESS;
	}
	else {
		questDB.dbState = DB_STATE::DELETE_FAILED;
	}

	if (player->_ownerRoom)
		player->_ownerRoom->DoAsync(&GameRoom::DB_Response_RemoveQuest, player, questDB);
}

void GameDBManager::Transact_UseItem(shared_ptr<Player> player, ItemDB itemDB)
{
	if (itemDB.dbState == DB_STATE::UPDATE_NEED) {
		SP::UpdateItem updateItem(*_dbConn);
		updateItem.In_ItemDbId(itemDB.ItemDbId);
		updateItem.In_PlayerDbId(itemDB.PlayerDbId);
		updateItem.In_TemplateId(itemDB.TemplateId);
		updateItem.In_Count(itemDB.Count);
		updateItem.In_Slot(itemDB.Slot);
		updateItem.In_Equipped(itemDB.Equipped);
		if (updateItem.Execute()) {
			itemDB.dbState = DB_STATE::UPDATE_SUCCESS;
		}
		else {
			itemDB.dbState = DB_STATE::UPDATE_FAILED;
		}
	}
	else if (itemDB.dbState == DB_STATE::DELETE_NEED) {
		SP::DeleteItem deleteItem(*_dbConn);
		deleteItem.In_ItemDbId(itemDB.ItemDbId);
		if (deleteItem.Execute()) {
			itemDB.dbState = DB_STATE::DELETE_SUCCESS;
		}
		else {
			itemDB.dbState = DB_STATE::DELETE_FAILED;
		}
	}
	else {

	}

	// 
	if (player->_ownerRoom)
		player->_ownerRoom->DoAsync(&GameRoom::DB_Response_UseItem, player, itemDB);
}

void GameDBManager::Transact_AddItem(shared_ptr<Player> player, ItemDB itemDB)
{
	// 인서트
	if (itemDB.dbState == DB_STATE::INSERT_NEED) {
		int64 identity;
		SP::InsertItemAndSelectIdentity insertItem(*_dbConn);
		insertItem.In_PlayerDbId(itemDB.PlayerDbId);
		insertItem.In_TemplateId(itemDB.TemplateId);
		insertItem.In_Count(itemDB.Count);
		insertItem.In_Slot(itemDB.Slot);
		insertItem.In_Equipped(itemDB.Equipped);
		insertItem.Out_Identity(identity);
		if (insertItem.Execute()) {
			if (insertItem.Fetch()) {
				itemDB.ItemDbId = identity;
				itemDB.dbState = DB_STATE::INSERT_SUCCESS;
			}
		}
		else {
			itemDB.dbState = DB_STATE::INSERT_FAILED;
		}
	}
	// 업데이트
	else if (itemDB.dbState == DB_STATE::UPDATE_NEED) {
		SP::UpdateItem updateItem(*_dbConn);
		updateItem.In_ItemDbId(itemDB.ItemDbId);
		updateItem.In_PlayerDbId(itemDB.PlayerDbId);
		updateItem.In_TemplateId(itemDB.TemplateId);
		updateItem.In_Count(itemDB.Count);
		updateItem.In_Slot(itemDB.Slot);
		updateItem.In_Equipped(itemDB.Equipped);
		if (updateItem.Execute()) {
			itemDB.dbState = DB_STATE::UPDATE_SUCCESS;
		}
		else {
			itemDB.dbState = DB_STATE::UPDATE_FAILED;
		}
	}
	// 
	else {

	}

	if (player->_ownerRoom)
		player->_ownerRoom->DoAsync(&GameRoom::DB_Response_AddItem, player, itemDB);
}

