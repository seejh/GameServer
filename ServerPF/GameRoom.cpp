#include "pch.h"
#include "GameRoom.h"

#include"GameObject.h"
#include"GameSession.h"
#include"GameClientPacketHandler.h"
#include"ObjectManager.h"
#include"Item.h"
#include"GameDBManager.h"
#include"DBManager.h"
#include"QuestManager.h"
#include"Quest.h"
#include"GameMap.h"

GameRoom::GameRoom()
{
	_gameMap = make_shared<GameMap>();
}

// �ϴ� �������� ������ �������� ���� ũ�⸦ ������ ����
void GameRoom::Init()
{
	// TODO : Config
	_gameMap->Load();

	
#pragma region SpawnMonsterAndNPC

	// ���߿� ����
	{// ����
		// 0�� ���� ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// ���̽� ��ġ, ��ġ
		monster->_basePos.CopyFrom(MakePFVector(2000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		// ���� ���ӷ� ����
		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(2000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(4000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(4000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(6000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(6000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos.CopyFrom(MakePFVector(8000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);
		
		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos.CopyFrom(MakePFVector(8000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);
		
		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(0, -5000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);
		
		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(2000, -5000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);
		
		EnterRoom(monster);
	}


	{ // NPC
		shared_ptr<Npc> npc = static_pointer_cast<Npc>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::NPC));
		npc->Init(1);

		npc->_info.mutable_pos()->mutable_location()->CopyFrom(MakePFVector(0, -3000, 110.149994f));
		npc->_info.mutable_pos()->mutable_rotation()->CopyFrom(MakePFVector(0, 0, 0));
		
		EnterRoom(npc);
	}
	{ // NPC
		shared_ptr<Npc> npc = static_pointer_cast<Npc>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::NPC));
		npc->Init(2);

		npc->_info.mutable_pos()->mutable_location()->CopyFrom(MakePFVector(0, 0, 110.149994f));
		npc->_info.mutable_pos()->mutable_rotation()->CopyFrom(MakePFVector(0, 0, -180.f));

		EnterRoom(npc);
	}
#pragma endregion
}

shared_ptr<Zone>& GameRoom::GetZone(float targetX, float targetY)
{
	int x = abs(_gameMap->_minX - targetX) / _gameMap->_zoneWidth;
	int y = abs(_gameMap->_minY - targetY) / _gameMap->_zoneHeight;

	return _gameMap->_zones[y][x];
}

void GameRoom::GetAdjacentZone(float targetX, float targetY, vector<shared_ptr<Zone>>& zones)
{
	int x = abs(_gameMap->_minX - targetX) / _gameMap->_zoneWidth;
	int y = abs(_gameMap->_minY - targetY) / _gameMap->_zoneHeight;

	for (int indexX = x - 1; indexX <= x + 1; indexX++) {
		if (indexX < 0 || indexX >= _gameMap->_zoneWidthCount)
			continue;

		for (int indexY = y - 1; indexY <= y + 1; indexY++) {
			if (indexY < 0 || indexY >= _gameMap->_zoneHeightCount)
				continue;

			zones.push_back(_gameMap->_zones[indexY][indexX]);
		}
	}
}

float GameRoom::GetDistance(float dX, float dY, float dZ)
{
	return sqrt(GetDistanceSimple(dX, dY, dZ));
}

float GameRoom::GetDistanceSimple(float dX, float dY, float dZ)
{
	return pow(abs(dX), 2) + pow(abs(dY), 2) + pow(abs(dZ), 2);
}

float GameRoom::GetDistanceXY(float dX, float dY)
{
	return sqrt(GetDistanceXYSimple(dX, dY));
}

float GameRoom::GetDistanceXYSimple(float dX, float dY)
{
	return pow(abs(dX), 2) + pow(abs(dY), 2);
}

void GameRoom::EnterRoom(shared_ptr<GameObject> object)
{
	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(object->_info.objectid());
	PROTOCOL::S_SPAWN spawnPkt;

	// �÷��̾�
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// 
		if (_gameMap->_players.count(object->_info.objectid()) == 0) {
			// �÷��̾�� ����ȯ
			shared_ptr<Player> player = static_pointer_cast<Player>(object);

			// cout << "[Room] SESSION-" << player->_ownerSession->_sessionId << ", PLAYER-" << object->_info.objectid() << " EnterRoom" << endl;
			
			// ��� �÷��̾�� ���� ���� �߰�
			_gameMap->_players[player->_info.objectid()] = player;
			player->_ownerRoom = static_pointer_cast<GameRoom>(shared_from_this());

			// �÷��̾� ��(��) �߰�
			shared_ptr<Zone>& zone = GetZone(player->_info.pos().location().x(), player->_info.pos().location().y());
			zone->_players.insert(player);

			// 
			player->OnEnterGame();

			return;
		}
		else {
			shared_ptr<Player> player = static_pointer_cast<Player>(object);
			// cout << "Room::EnterRoom() SESSION-" << player->_ownerSession->_sessionId << ", _players.count(ObjectId-" << object->_info.objectid() << ") No Remove Error" << endl;
		}
	}
	// ����
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		 
		if (_gameMap->_monsters.count(object->_info.objectid()) == 0) {
			cout << "[Room] MONSTER-" << object->_info.objectid() << " EnterRoom" << endl;

			// ���ͷ� ����ȯ
			shared_ptr<Monster> monster = static_pointer_cast<Monster>(object);

			// ��� ���Ϳ��� ���� ���� �߰�
			_gameMap->_monsters[monster->_info.objectid()] = monster;
			monster->_ownerRoom = static_pointer_cast<GameRoom>(shared_from_this());

			// ���� ��(��)�� �߰�
			shared_ptr<Zone> zone = GetZone(monster->_info.pos().location().x(), monster->_info.pos().location().y());
			zone->_monsters.insert(monster);

			// ���� ��ε� ĳ��Ʈ
			spawnPkt.add_object()->CopyFrom(monster->_info);

			// 
			monster->OnEnterGame();
		}
	}

	else if (type == PROTOCOL::GameObjectType::NPC) {
		if (_gameMap->_npcs.count(object->_info.objectid()) == 0) {
			cout << "[Room] NPC-" << object->_info.objectid() << " EnterRoom" << endl;

			// NPC�� ����ȯ
			shared_ptr<Npc> npc = static_pointer_cast<Npc>(object);

			// ��� npc���� ���� ���� �߰�
			_gameMap->_npcs[npc->_info.objectid()] = npc;
			npc->_ownerRoom = static_pointer_cast<GameRoom>(shared_from_this());

			// npc ��(��)�� �߰�
			shared_ptr<Zone> zone = GetZone(npc->_info.pos().location().x(), npc->_info.pos().location().y());
			zone->_npcs.insert((npc));

			// ���� ��ε� ĳ��Ʈ
			spawnPkt.add_object()->CopyFrom(npc->_info);

			//
			npc->OnEnterGame();
		}
	}

	// ����ü, ��Ÿ���...
	else {
		return;
	}
}

void GameRoom::LeaveRoom(int objectId)
{
	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(objectId);
	PROTOCOL::S_DESPAWN despawnPkt;

	// �÷��̾�
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// cout << "[Room] PLAYER-" << objectId << " LeaveRoom" << endl;

		auto it = _gameMap->_players.find(objectId);
		if (it != _gameMap->_players.end()) {
			// 
			shared_ptr<Player> player = it->second;
			
			cout << "[Room] PLAYER-" << objectId << " LeaveRoom" << endl;

			// ��(��)���� �÷��̾� ����
			shared_ptr<Zone>& zone = GetZone(player->_info.pos().location().x(), player->_info.pos().location().y());
			zone->Remove(player);

			// �뿡�� �÷��̾� ����
			if (_gameMap->_players.erase(objectId) == 0) {
				// cout << "Room::LeaveRoom() Erase Player From _players Couldn't Error" << endl;
			}

			// �÷��̾� ���� ����(DB)
			player->OnLeaveGame();

			// ������Ʈ �Ŵ������� �÷��̾� ����
			ObjectManager::Instance()->Remove(player->_info.objectid());

			// ���ǿ��� �÷��̾� ���� - ���� ������ �� ���� ��� Ȯ�� �ʿ�
			player->_ownerSession->_player.reset();

			return;
		}
	}
	// ����
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		cout << "[Room] MONSTER-" << objectId << " LeaveRoom" << endl;

		auto it = _gameMap->_monsters.find(objectId);
		if (it != _gameMap->_monsters.end()) {
			// 
			shared_ptr<Monster> monster = it->second;

			// ��(��)���� ���� ����
			shared_ptr<Zone>& zone = GetZone(monster->_info.pos().location().x(), monster->_info.pos().location().y());
			zone->Remove(monster);

			// �뿡�� ���� ����
			_gameMap->_monsters.erase(objectId);

			monster->OnLeaveGame();
		}
	}
	// NPC
	else if (type == PROTOCOL::GameObjectType::NPC) {
		cout << "[Room] NPC-" << objectId << " LeaveRoom" << endl;

		auto it = _gameMap->_npcs.find(objectId);
		if (it != _gameMap->_npcs.end()) {
			//
			shared_ptr<Npc> npc = it->second;

			// ��(��)���� NPC ����
			shared_ptr<Zone>& zone = GetZone(npc->_info.pos().location().x(), npc->_info.pos().location().y());
			zone->Remove(npc);

			// �뿡�� npc ����
			_gameMap->_npcs.erase(objectId);

			//
			npc->OnLeaveGame();
		}
	}
	// ����ü, ��Ÿ���
	else {
		return;
	}
}

/*-----------------------------------------------------------------------------
	��Ŷ �ڵ�
---------------------------------------------------------------------------*/


void GameRoom::HandleUseItem(shared_ptr<Player> player, PROTOCOL::C_USE_ITEM fromPkt)
{
	// ������ ��ȸ
	shared_ptr<Item> item = player->_inven.GetItemBySlot(fromPkt.slot());
	if (item == nullptr)
		return;

	// �Һ�������Ѵ�.
	if (item->_itemType != PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE)
		return;

	// ���� ��
	if (player->_inven.SetSlotDBLock(fromPkt.slot(), true) == false) {
		cout << "Room::HandleUseItem() Error - " << fromPkt.slot() << "Slot Locked Already" << endl;
		return;
	}
		
	// DB ��û �غ�
	ItemDB itemDB;
	itemDB.ItemDbId = item->_itemInfo.itemdbid();
	itemDB.TemplateId = item->_itemInfo.templateid();
	itemDB.PlayerDbId = item->_itemInfo.playerdbid();
	itemDB.Count = item->_itemInfo.count() - 1;
	itemDB.Slot = item->_itemInfo.slot();
	itemDB.Equipped = item->_itemInfo.equipped();

	// �Һ� �� ������ �ȵǸ� => 1�����Ҵµ� ��� -> ����
	// �Һ� �� ������ �Ǹ� => 100�� ���Ҵµ� ��� -> 99�� -> ����
	
	// ���� (DELETE) or ���� (UPDATE)
	if (item->_itemInfo.count() <= 1)
		itemDB.dbState = DB_STATE::DELETE_NEED;
	else 
		itemDB.dbState = DB_STATE::UPDATE_NEED;

	// DB ��û
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_UseItem, player, itemDB);
}

void GameRoom::HandleEquipItem(shared_ptr<Player> player, PROTOCOL::C_EQUIP_ITEM fromPkt)
{
	// ������ ��ȸ
	shared_ptr<Item> item = player->_inven.GetItemBySlot(fromPkt.slot());
	if (item == nullptr)
		return;

	// ���������Ѵ�.
	if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE)
		return;

	// ������ ���(��û�� ���� ��û && �ش� ������ �̹� ������ ��� ������)
	if (fromPkt.equip() == true) {
		shared_ptr<Item> equippedItem = player->_inven.FindEquippedSamePos(item);
		if (equippedItem != nullptr) {
			// ���� ��
			if (player->_inven.SetSlotDBLock(equippedItem->_itemInfo.slot(), true) == false) {
				cout << "Room::HandleEquipItem() Error - Equipment to UnEquip, Slot:" << equippedItem->_itemInfo.slot() << ", SlotLocked Already" << endl;
				return;
			}

			// ���� �޸� ��� ����
			equippedItem->_itemInfo.set_equipped(false);

			// DB ��Ƽ
			DBManager::Instance()->_gameDbManager->TransactNoti_EquipItem(player, equippedItem);

			// ��Ŷ
			PROTOCOL::S_EQUIP_ITEM toPkt;
			toPkt.set_equip(equippedItem->_itemInfo.equipped());
			toPkt.set_slot(equippedItem->_itemInfo.slot());
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);

			// ���� �� ����
			player->_inven.SetSlotDBLock(equippedItem->_itemInfo.slot(), false);
		}
	}

	// ��û ���� ���� �Ǵ� ���� ����
	// ���� ��
	if (player->_inven.SetSlotDBLock(item->_itemInfo.slot(), true) == false) {
		cout << "Room::HandleEquipItem() Error - Equipment to Equip, Slot:" << item->_itemInfo.slot() << ", SlotLocked Already" << endl;
		return;
	}

	// ���� �޸� ��� ����
	item->_itemInfo.set_equipped(fromPkt.equip());

	// DB ��Ƽ
	DBManager::Instance()->_gameDbManager->TransactNoti_EquipItem(player, item);

	// ��Ŷ
	PROTOCOL::S_EQUIP_ITEM toPkt;
	toPkt.set_equip(item->_itemInfo.equipped());
	toPkt.set_slot(item->_itemInfo.slot());
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	player->_ownerSession->SendPacket(sendBuffer);

	// ���� �� ����
	player->_inven.SetSlotDBLock(item->_itemInfo.slot(), false);
}

void GameRoom::HandleAddQuest(shared_ptr<Player> player, PROTOCOL::C_ADD_QUEST fromPkt)
{
	// �ش� ���� �̹� ����Ʈ�� ������ �н�
	// ���⼭ �ð� ���� ��ƸԾ���. �������̳ʿ��� []�� ������ �� nullptr�� ���� ����
	// shared_ptr<Quest> quest = player->_questManager->_quests[fromPkt.quest().templateid()];
	auto it = player->_questManager->_quests.find(fromPkt.quest().templateid());
	if (it != player->_questManager->_quests.end()) {
		cout << "Room::HandleAddQuest() Error - QuestTemplateId:" << fromPkt.quest().templateid() << " is Already Exists" << endl;
		return;
	}

	// DB ��û �غ�
	QuestDB questDB;
	questDB.TemplateId = fromPkt.quest().templateid();
	questDB.PlayerDbId = player->_info.playerdbid();
	questDB.Progress = 0;
	questDB.Completed = false;
	questDB.dbState = DB_STATE::INSERT_NEED;

	// DB ��û
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_AddQuest, player, questDB);
}

void GameRoom::HandleRemoveQuest(shared_ptr<Player> player, PROTOCOL::C_REMOVE_QUEST fromPkt)
{
	// ����Ʈ ��ȸ
	shared_ptr<Quest> quest = player->_questManager->_quests[fromPkt.questdbid()]; 
	if (quest == nullptr) {
		cout << "Room::HandleRemoveQuest() Error - Can't Find QuestId: " << fromPkt.questdbid() << endl;
		return;
	}
	
	// DB ��û �غ�
	QuestDB questDB;
	questDB.QuestDbId = quest->_questInfo.questdbid();
	questDB.TemplateId = quest->_questInfo.templateid();
	questDB.PlayerDbId = quest->_questInfo.playerdbid();
	questDB.Progress = quest->_questInfo.progress();
	questDB.Completed = quest->_questInfo.completed();
	questDB.dbState = DB_STATE::DELETE_NEED;

	// DB ��û
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_RemoveQuest, player, questDB);
}

void GameRoom::HandleCompleteQuest(shared_ptr<Player> player, PROTOCOL::C_COMPLETE_QUEST fromPkt)
{
	// ����Ʈ ��ȸ
	auto it = player->_questManager->_quests.find(fromPkt.questdbid());
	if (it == player->_questManager->_quests.end())
		return;

	// ����Ʈ �Ϸ� ���� Ȯ��
	if (it->second->_questInfo.progress() < it->second->_questData->quantity)
		return;

	// ����Ʈ ���� ������ ��� ����
	vector<ItemDB> itemDBs;
	for (auto p : it->second->_questData->rewardItems) {
		ItemDB itemDB = player->CanAddItemDB(p.first, p.second);
		if (itemDB.Slot == -1) {
			// ���� �� ����
			for (ItemDB itemdb : itemDBs)
				player->_inven.SetSlotDBLock(itemdb.Slot, false);

			return;
		}

		itemDBs.push_back(itemDB);
	}

	// ����Ʈ ����
	QuestDB questDB;
	questDB.QuestDbId = it->second->_questInfo.questdbid();
	questDB.TemplateId = it->second->_questInfo.templateid();
	questDB.Progress = it->second->_questInfo.progress();
	questDB.Completed = true;
	questDB.PlayerDbId = it->second->_questInfo.playerdbid();

	//  DB ��û
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_CompleteQuest, player, questDB, itemDBs);
}

void GameRoom::HandleUpdateQuest(shared_ptr<Player> player, PROTOCOL::C_UPDATE_QUEST fromPkt)
{
	// ��ȣ �ۿ� ������Ʈ
	
	// ���ӿ�����Ʈ ��ȸ
	shared_ptr<GameObject> object  = static_pointer_cast<Npc>(ObjectManager::Instance()->_objects[fromPkt.objectid()]);
	if (object == nullptr)
		return;

	// NPC�� �ƴϸ� �н�
	if (object->_info.objecttype() != PROTOCOL::GameObjectType::NPC)
		return;

	// �Ÿ� Ȯ�� �� ���� - �ϴ� �н�

	// ������Ʈ
	player->UpdateQuest(QuestType::QUEST_TYPE_INTERACT, object->_info.typetemplateid(), fromPkt.questinfo().progress());
}



/*-----------------------------------------------------------------------------
	DB
---------------------------------------------------------------------------*/

void GameRoom::DB_Response_UseItem(shared_ptr<Player> player, ItemDB itemDB)
{
	// ����, ������Ʈ ���� (DB) -> ����, ������Ʈ (����) -> ���Զ� ���� -> ������ ȿ�� -> ��Ŷ
	// ����, ������Ʈ, �� �� ���� (DB) -> ���Զ� ����
	
	// ����
	if (itemDB.dbState != DB_STATE::UPDATE_SUCCESS && itemDB.dbState != DB_STATE::DELETE_SUCCESS) {
		cout << "Room::DBCallback_UseItem() Error - dbState Not UPDATE_SUCCESS & dbState Not DELETE_SUCCESS" << endl;
		player->_inven.SetSlotDBLock(itemDB.Slot, false);
		return;
	}

	// ����
	// ������ ��ȸ
	shared_ptr<Item> item = player->_inven.GetItemBySlot(itemDB.Slot);
	if (item == nullptr) {
		cout << "Room::DBCallback_UseItem() Error - Can't Find ItemSlot:" << itemDB.Slot << endl;
		return;
	}
		
	// ������Ʈ ����
	if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		// �κ��丮���� ������ ������Ʈ
		item->_itemInfo.set_count(itemDB.Count);

		// ��� ��Ŷ
		PROTOCOL::S_USE_ITEM toPkt;
		toPkt.mutable_item()->set_slot(itemDB.Slot);
		toPkt.mutable_item()->set_count(itemDB.Count);
		
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}

	// ����Ʈ ����
	else if (itemDB.dbState == DB_STATE::DELETE_SUCCESS) {
		// �κ��丮���� ������ ����
		player->_inven.Remove(itemDB.Slot);

		// ���� ��Ŷ
		PROTOCOL::S_REMOVE_ITEM removePkt;
		removePkt.add_slots(itemDB.Slot);

		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(removePkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	
	// ������ ��� ȿ��
	player->UseItem(item->_itemInfo.templateid());

	// ���� �� ����
	player->_inven.SetSlotDBLock(itemDB.Slot, false);
}

void GameRoom::DB_Response_AddItem(shared_ptr<Player> player, ItemDB itemDB)
{
	player->AddItem(itemDB);
}

// DB - ����Ʈ
void GameRoom::DB_Response_RemoveQuest(shared_ptr<Player> player, QuestDB questDB)
{
	if (questDB.dbState == DB_STATE::DELETE_SUCCESS) {
		player->RemoveQuest(questDB);
	}
}

void GameRoom::DB_Response_AddQuest(shared_ptr<Player> player, QuestDB questDB)
{
	if (questDB.dbState == DB_STATE::INSERT_SUCCESS) {
		player->AddQuest(questDB);
	}
}

void GameRoom::DB_Response_CompleteQuest(shared_ptr<Player> player, QuestDB questDB, vector<ItemDB> itemDBs)
{
	// ����
	if (questDB.Completed == true) {

		// ����Ʈ ������Ʈ
		player->CompleteQuest(questDB);

		// ������ ����
		for (ItemDB& itemDB : itemDBs) 
			player->AddItem(itemDB);

		// Exp ������Ʈ
		player->AddExp(DataManager::Instance()->_questTable[questDB.TemplateId]->rewardExp);
	}

	// ����
	else {

	}

	// ���� �� ����
	for (ItemDB& itemdb : itemDBs)
		player->_inven.SetSlotDBLock(itemdb.Slot, false);
}

void GameRoom::Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer)
{
	// ��ε�ĳ��Ʈ�� �� ����
	vector<shared_ptr<Zone>> zones;
	GetAdjacentZone(locX, locY, zones);

	// ������ ���� ��� �÷��̾�� ��ε�ĳ����
	for (shared_ptr<Zone>& z : zones) 
		for (shared_ptr<Player> p : z->_players) 
			p->_ownerSession->SendPacket(sendBuffer);

}

void GameRoom::BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer)
{
	// ��ε�ĳ��Ʈ�� �� ����
	vector<shared_ptr<Zone>> zones;
	GetAdjacentZone(player->_info.pos().location().x(), player->_info.pos().location().y(), zones);

	// ������ ������ ���� ���� ��� �÷��̾�� ��ε�ĳ����
	for (shared_ptr<Zone>& z : zones) {
		for (shared_ptr<Player> p : z->_players) {
			if (player->_info.objectid() == p->_info.objectid())
				continue;

			p->_ownerSession->SendPacket(sendBuffer);
		}
	}
}

void GameRoom::ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_SKILL fromPkt)
{
	// ��Ÿ�� �� üũ OK - �ϴ� �н�
	
	// ������ ��ġ�� ������� ���� ��Ŷ ��ε�ĳ����
	shared_ptr<GameObject> attacker = ObjectManager::Instance()->Find(fromPkt.attacker());
	if (attacker == nullptr)
		return;

	// ���� ��Ŷ
	PROTOCOL::S_SKILL toPkt;
	toPkt.set_attacker(fromPkt.attacker());
	toPkt.set_skillid(fromPkt.skillid());
	for (int victim : fromPkt.victims())
		toPkt.add_victims(victim);
	
	// ���� ��ε�ĳ����
	shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(attacker->_info.pos().location().x(), attacker->_info.pos().location().y(), sendBuffer);


	// ��ų ����
	// ��ų ���� Ȯ��
	auto it = DataManager::Instance()->_skillTable.find(fromPkt.skillid());
	if (it != DataManager::Instance()->_skillTable.end()) {
		// ��ų ����
		SkillData skillData = it->second;
		
		// ��ų ������ ���� ó��
		switch (skillData.skillType) {
		// ��Ÿ
		case PROTOCOL::SkillType::SKILL_AUTO: 
		{
			// ����ڵ�
			for (int i = 0; i < fromPkt.victims_size(); i++) {
				// 
				shared_ptr<GameObject> victim = ObjectManager::Instance()->Find(fromPkt.victims(i));
				if (victim == nullptr) 
					continue;

				float dx = gameObject->_info.pos().location().x() - victim->_info.pos().location().x();
				float dy = gameObject->_info.pos().location().y() - victim->_info.pos().location().y();

				// TODO : ����, ��Ÿ� �ȿ� �����
				float simpleDT = GetDistanceXYSimple(
					victim->_info.pos().location().x() - gameObject->_info.pos().location().x(),
					victim->_info.pos().location().y() - gameObject->_info.pos().location().y());
				if (simpleDT <= pow(180, 2)) {
					
					// �Ѱ��ݷ� + ��ų �����
					// �÷��̾� = ������ ����� + ���� ����� + ��ų �����
					// ���� = ���� ����� + ��ų �����
					victim->OnDamaged(attacker, attacker->_totalDamage + skillData.damage);
				}
			}
		} 
		break;

		// �� ��
		default:
			break;
		}
	}
}

void GameRoom::PlayerChat(shared_ptr<Player> player, PROTOCOL::C_CHAT fromPkt)
{
	// ��Ŷ ����
	PROTOCOL::S_CHAT toPkt;
	toPkt.mutable_object()->set_name(player->_info.name());
	toPkt.set_text(fromPkt.text());

	// ��ε�ĳ��Ʈ
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(player->_info.pos().location().x(), player->_info.pos().location().y(), sendBuffer);
}

float GameRoom::GetPlayerAround(shared_ptr<Monster> monster)
{
	// ������ ��ġ
	const float monsterX = monster->_info.pos().location().x();
	const float monsterY = monster->_info.pos().location().y();

	// ���� Ž�� ���� - (����, ��Ŭ����x,)
	float dtMonsterNoticeSimple = pow(monster->_info.stat().noticedistance(), 2);
	
	// Ž���� �� = �ش� ���� �ֺ� ����
	vector<shared_ptr<Zone>> zones;
	GetAdjacentZone(monsterX, monsterY, zones);

	// Ž���� ������ �÷��̾� ����
	for (shared_ptr<Zone>& z : zones) {
		//
		for (shared_ptr<Player> p : z->_players) {
			// ���Ϳ� �÷��̾� �� �Ÿ��� ���� ª�� �༮���� ����
			float dtToTargetSimple = GetDistanceXYSimple(
				p->_info.pos().location().x() - monsterX, p->_info.pos().location().y() - monsterY);

			// ���� Ž�� ����(����)�� �����÷��̾�Ÿ�(����) ��
			if (dtToTargetSimple <= dtMonsterNoticeSimple) {
				// ���� Ÿ�� ����
				monster->_target = p;

				// ������ ��ȯ
				return sqrt(dtToTargetSimple);
			}
		}
	}
	
	// �÷��̾� ������ ������ ����
	return -1.f;
}

PROTOCOL::PFVector GameRoom::CalculatePathFind(shared_ptr<Monster>& monster, vector<PROTOCOL::PFVector>& paths)
{
	// �̵� ���� �Ÿ�
	// float deltaTime = (float)(monster->_moveDelta / 1000.f);
	// monster->_moveDelta = 0;
	
	float deltaTime = (float)(monster->_nowUpdateTime - monster->_lastUpdateTime) / 1000.f;
	float dtMovable = deltaTime * monster->_info.stat().speed();

	// �� ��ġ
	PROTOCOL::PFVector nowPos = monster->_info.pos().location();

	// �̵� Ȯ�� ����
	PROTOCOL::PFVector destPos;
	
	// ��� ���� �� ���
	for (int i = 0; i < paths.size(); i++) {
		// �� ��ġ���� ���� ��ġ���� �̵� ����
		PROTOCOL::PFVector moveV;
		SubV(&moveV, &paths[i], &nowPos);

		// ���� ũ��
		float dt = GetDistance(moveV.x(), moveV.y(), moveV.z());

		// ���� �̵� ���� �Ÿ� - ���� �Ÿ�
		float perDT = dtMovable - dt;

		// �� �̵� ����
		if (perDT >= 0) {
			// ��ġ �̵�, �̵� ���� �Ÿ� ������Ʈ
			nowPos = paths[i];
			destPos = nowPos;
			
			// ���� �̵� �Ÿ� ������Ʈ
			dtMovable -= dt;
		}

		// �̵� �Ұ�
		else {
			// ���� * ���� �̵� �Ÿ�
			Normalize(&moveV);
			moveV.set_x(moveV.x() * dtMovable);
			moveV.set_y(moveV.y() * dtMovable);
			moveV.set_z(moveV.z() * dtMovable);

			// ���� ��ġ ���� = �� ��ġ + �̵� ����
			SumV(&nowPos, &nowPos, &moveV);
			destPos = nowPos;

			break;
		}
	}

	return destPos;
}

void GameRoom::ActorMove(shared_ptr<GameObject> gameObject, PROTOCOL::PFVector destPos)
{
	// �� ��ġ
	float nowX = gameObject->_info.pos().location().x();
	float nowY = gameObject->_info.pos().location().y();
	shared_ptr<Zone>& nowZone = GetZone(nowX, nowY);

	// �̵� ��ġ
	float afterX = destPos.x();
	float afterY = destPos.y();
	shared_ptr<Zone>& afterZone = GetZone(afterX, afterY);

	// �� ��ȣ�� �ٸ��� �� ����
	if (nowZone->_zoneId != afterZone->_zoneId) {
		afterZone->Add(gameObject);
		nowZone->Remove(gameObject);
	}

	// ��ġ ����
	gameObject->_info.mutable_pos()->mutable_location()->CopyFrom(destPos);
	
	// ��ε�ĳ��Ʈ
	// ��Ŷ
	PROTOCOL::S_MOVE toPkt;
	toPkt.mutable_object()->CopyFrom(gameObject->_info);

	// ����
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(afterX, afterY, sendBuffer);
}

bool GameRoom::FindPath(shared_ptr<Monster> monster, PROTOCOL::PFVector ep, PROTOCOL::PFVector& destPos)
{
	vector<PROTOCOL::PFVector> paths;
	if (_gameMap->FindPath(monster, ep, paths) == false)
		return false;

	destPos = CalculatePathFind(monster, paths);

	return true;
}

bool GameRoom::FindRandomPos(shared_ptr<Monster> monster)
{
	PROTOCOL::PFVector destPos;
	if (_gameMap->FindRandomPos(monster, destPos) == false)
		return false;

	monster->_patrolPos = destPos;

	return true;
}

