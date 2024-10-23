#include "pch.h"
#include "GameDBManager.h"

#include"GameObject.h"
#include"Room.h"
#include"DataManager.h"
#include"DBConnection.h"
#include"DBConnectionPool.h"
#include"DBDataModel.h"
#include"GenProcedures.h"
#include"Item.h"
#include"Inventory.h"


//bool GameDBManager::Connect(int32 connectionCounts, const WCHAR* connectionString)
//{
//	_connectionCounts = connectionCounts + 1;
//	if (GDBConnectionPool->Connect(connectionCounts, connectionString) == false)
//		return false;
//	
//	_dbConn = Pop();
//	if (_dbConn == nullptr)
//		return false;
//
//	return true;
//}
//
//DBConnection* GameDBManager::Pop()
//{
//	lock_guard<mutex> lock(_m);
//
//	return GDBConnectionPool->Pop();
//}
//
//void GameDBManager::Push(DBConnection* dbConn)
//{
//	lock_guard<mutex> lock(_m);
//
//	GDBConnectionPool->Push(dbConn);
//}

//////////////////////////////////////////////////////////
// 잡큐 - 

bool GameDBManager::Connect(int32 connectionCounts, const WCHAR* connectionString)
{
	_connectionCounts = connectionCounts + 1;

	if (GDBConnectionPool->Connect(connectionCounts, connectionString) == false)
		return false;

	_dbConn = Pop();
	if (_dbConn == nullptr)
		return false;

	return true;
}

DBConnection* GameDBManager::Pop()
{
	return GDBConnectionPool->Pop();
}

void GameDBManager::Push(DBConnection* dbConn)
{
	GDBConnectionPool->Push(dbConn);
}


/*-------------------------------------------------------------------------------
	UPDATE, SELECt, INSERT, DELETE
--------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------
	아이템
----------------------------------------------------------------------------------*/
void GameDBManager::InsertItem(shared_ptr<Player> player, ItemDB itemDB, void(Room::* memFunc)(shared_ptr<Player>, ItemDB))
{
	string result;

	// DB 작업
	int64 identity;
	SP::InsertItemAndSelectIdentity insertItem(*_dbConn);
	insertItem.In_TemplateId(itemDB.TemplateId);
	insertItem.In_PlayerDbId(itemDB.PlayerDbId);
	insertItem.In_Count(itemDB.Count);
	insertItem.In_Slot(itemDB.Slot);
	insertItem.In_Equipped(itemDB.Equipped);
	insertItem.Out_Identity(identity);
	if (insertItem.Execute()) {
		if (insertItem.Fetch()) {
			itemDB.ItemDbId = identity;
			itemDB.dbState = DB_STATE::INSERT_SUCCESS;

			result = ", INSERT_SUCCESS";
		}
		else {
			result = ", INSERT_FAILED - Execute OK But Fetch Error";
			itemDB.dbState = DB_STATE::INSERT_FAILED; // Fetch가 실패가 나는 것이 DB 작업이 실패인지 확인할 필요가 있다, 일단은 실패 판정
		}
	}
	else {
		itemDB.dbState = DB_STATE::INSERT_FAILED;
		result = ", INSERT_FAILED";
	}

	// 로그
	cout << "[DB_INSERT_ITEM] - ItemDbId" << itemDB.ItemDbId << ", Slot:" << itemDB.Slot << result << endl;

	// 요청한 곳에 콜백으로 넘겨줌
	if (player->_ownerRoom != nullptr)
		player->_ownerRoom->DoAsync(memFunc, player, itemDB);
}

void GameDBManager::UpdateItem(shared_ptr<Player> player, ItemDB itemDB, void(Room::* memFunc)(shared_ptr<Player>, ItemDB))
{
	// DB 작업
	SP::UpdateItem updateItem(*_dbConn);
	updateItem.In_ItemDbId(itemDB.ItemDbId);
	updateItem.In_TemplateId(itemDB.TemplateId);
	updateItem.In_PlayerDbId(itemDB.PlayerDbId);
	updateItem.In_Count(itemDB.Count);
	updateItem.In_Slot(itemDB.Slot);
	updateItem.In_Equipped(itemDB.Equipped);
	if (updateItem.Execute())
		itemDB.dbState = DB_STATE::UPDATE_SUCCESS;
	else
		itemDB.dbState = DB_STATE::UPDATE_FAILED;

	//로그
	cout << "[DB_UPDATE_ITEM] - ItemDbId:" << itemDB.ItemDbId << ", Slot:" << itemDB.Slot << ", Result:"
		<< boolalpha << (itemDB.dbState == DB_STATE::UPDATE_SUCCESS ? "UPDATE_SUCCESS" : "UPDATE_FAILED") << endl;

	// 요청한 곳에 콜백으로 넘겨줌
	if (player->_ownerRoom != nullptr)
		player->_ownerRoom->DoAsync(memFunc, player, itemDB);
}

void GameDBManager::DeleteItem(shared_ptr<Player> player, ItemDB itemDB, void(Room::* memFunc)(shared_ptr<Player>, ItemDB))
{
	// DB 작업
	SP::DeleteItem deleteItem(*_dbConn);
	deleteItem.In_ItemDbId(itemDB.ItemDbId);
	if (deleteItem.Execute())
		itemDB.dbState = DB_STATE::DELETE_SUCCESS;
	else
		itemDB.dbState = DB_STATE::DELETE_FAILED;

	// 로그
	cout << "[DB_DELETE_ITEM] - ItemDbId:" << itemDB.ItemDbId << ", Slot:" << itemDB.Slot << ", Result:"
		<< boolalpha << (itemDB.dbState == DB_STATE::DELETE_SUCCESS ? "DELETE_SUCCESS" : "DELETE_FAILED") << endl;

	// 요청한 곳에 콜백으로 넘겨줌
	if (player->_ownerRoom != nullptr)
		player->_ownerRoom->DoAsync(memFunc, player, itemDB);
}


/*---------------------------------------------------------------------------------
	퀘스트
----------------------------------------------------------------------------------*/
void GameDBManager::InsertQuest(shared_ptr<Player> player, QuestDB questDB, void(Room::* memFunc)(shared_ptr<Player>, QuestDB))
{
	string result;

	// DB 작업
	int64 identity;
	SP::InsertQuestAndSelectIdentity insertQuest(*_dbConn);
	insertQuest.In_TemplateId(questDB.TemplateId);
	insertQuest.In_PlayerDbId(questDB.PlayerDbId);
	insertQuest.In_Progress(questDB.Progress);
	insertQuest.In_Completed(questDB.Completed);
	insertQuest.Out_Identity(identity);
	if (insertQuest.Execute()) {
		if (insertQuest.Fetch()) {
			// 생성 후 DB 인덱스
			questDB.QuestDbId = identity;

			result = " Result: INSERT_SUCCESS";
			questDB.dbState = DB_STATE::INSERT_SUCCESS;
		}
		else {
			result = " Result: INSERT_FAILED (Execute OK But Fetch Failed)";
			questDB.dbState = DB_STATE::INSERT_FAILED;
		}
	}
	else {
		result = " Result: INSERT_FAILED";
		questDB.dbState = DB_STATE::INSERT_FAILED;
	}

	// 로그
	cout << "[DB_INSERT_QUEST] - QuestDbId:" << questDB.QuestDbId << ", TmeplateId:" << questDB.TemplateId <<
		result << endl;

	// 요청한 곳에 콜백으로 넘겨줌
	if (player->_ownerRoom != nullptr)
		player->_ownerRoom->DoAsync(memFunc, player, questDB);
}

void GameDBManager::UpdateQuest(shared_ptr<Player> player, QuestDB questDB, void(Room::* memFunc)(shared_ptr<Player>, QuestDB))
{
	// DB 작업
	SP::UpdateQuest updateQuest(*_dbConn);
	updateQuest.In_QuestDbId(questDB.QuestDbId);
	updateQuest.In_TemplateId(questDB.TemplateId);
	updateQuest.In_PlayerDbId(questDB.PlayerDbId);
	updateQuest.In_Progress(questDB.Progress);
	updateQuest.In_Completed(questDB.Completed);
	if (updateQuest.Execute())
		questDB.dbState = DB_STATE::UPDATE_SUCCESS;
	else
		questDB.dbState = DB_STATE::UPDATE_FAILED;

	// 로그
	cout << "[DB_UPDATE_QUEST] - QuestDbId:" << questDB.QuestDbId << ", TemplateId:" << questDB.TemplateId <<
		(questDB.dbState == DB_STATE::UPDATE_SUCCESS ? " Result: UPDATE_SUCCESS" : " Result: UPDATE_FAILED") << endl;

	// 요청한 곳에 콜백으로 넘겨줌
	if (player->_ownerRoom != nullptr)
		player->_ownerRoom->DoAsync(memFunc, player, questDB);
}

void GameDBManager::DeleteQuest(shared_ptr<Player> player, QuestDB questDB, void(Room::* memFunc)(shared_ptr<Player>, QuestDB))
{
	// DB 작업
	SP::DeleteQuest deleteQuest(*_dbConn);
	deleteQuest.In_QuestDbId(questDB.QuestDbId);
	if (deleteQuest.Execute())
		questDB.dbState = DB_STATE::DELETE_SUCCESS;
	else
		questDB.dbState = DB_STATE::DELETE_FAILED;

	// 로그
	cout << "[DB_DELETE_QUEST] - QuestDbId:" << questDB.QuestDbId << ", TemplateId:" << questDB.TemplateId <<
		(questDB.dbState == DB_STATE::DELETE_SUCCESS ? " Result: DELETE_SUCCESS" : " Result: DELETE_FAILED") << endl;

	// 요청한 곳에 콜백으로 넘겨줌
	if (player->_ownerRoom != nullptr)
		player->_ownerRoom->DoAsync(memFunc, player, questDB);
}
