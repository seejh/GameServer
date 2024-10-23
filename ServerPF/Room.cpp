#include "pch.h"
#include "Room.h"

#include"GameObject.h"
#include"ClientSession.h"
#include"ClientPacketHandler.h"
#include"ObjectManager.h"
#include"Item.h"
#include"GameDBManager.h"
#include"DBManager.h"
#include"QuestManager.h"
#include"Quest.h"

Room::~Room()
{
	for (vector<Zone*> v : _zones)
		for (Zone* z : v)
			delete(z);
}

// �ϴ� �������� ������ �������� ���� ũ�⸦ ������ ����
void Room::Init(int minX, int maxX, int minY, int maxY)
{
	_minX = minX;
	_minY = minY;

	int sizeX = abs(maxX - minX);
	int sizeY = abs(maxY - minY);

	_countX = sizeX / _width;
	_countY = sizeY / _width;

	//
	_zones.resize(_countY);
	for (int y = 0; y < _countY; y++) {
		
		_zones[y].resize(_countX);
		for (int x = 0; x < _countX; x++) {

			int baseX = minX + (_width * x);
			int baseY = minY + (_width * y);
			
			// cout << "[" << y << "," << x << "] = [" << baseX << "," << baseY << "]" << endl;
			_zones[y][x] = new Zone(baseX, baseY, _width);
		}
	}
	
#pragma region SpawnMonsterAndNPC
	// ���߿� ����
	{// ����
		// 0�� ���� ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// ���̽� ��ġ, ��ġ
		monster->_basePos._x = 2000;
		monster->_basePos._y = -3000;
		monster->_basePos._z = 145.f;
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		// ���� ���ӷ� ����
		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos._x = 4000;
		monster->_basePos._y = 0;
		monster->_basePos._z = 145.f;
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos._x = 6000;
		monster->_basePos._y = -3000;
		monster->_basePos._z = 145.f;
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		EnterRoom(monster);
	}
	{// ����
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos._x = 8000;
		monster->_basePos._y = 0;
		monster->_basePos._z = 145.f;
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		EnterRoom(monster);
	}
	{ // NPC
		shared_ptr<Npc> npc = static_pointer_cast<Npc>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::NPC));
		npc->Init(1);

		npc->_info.mutable_pos()->set_locationx(0);
		npc->_info.mutable_pos()->set_locationy(-3000);
		npc->_info.mutable_pos()->set_locationz(110.149994f);

		npc->_info.mutable_pos()->set_rotationpitch(0);
		npc->_info.mutable_pos()->set_rotationroll(0);
		npc->_info.mutable_pos()->set_rotationyaw(0);

		EnterRoom(npc);
	}
	{ // NPC
		shared_ptr<Npc> npc = static_pointer_cast<Npc>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::NPC));
		npc->Init(2);

		npc->_info.mutable_pos()->set_locationx(0);
		npc->_info.mutable_pos()->set_locationy(0);
		npc->_info.mutable_pos()->set_locationz(110.149994f);

		npc->_info.mutable_pos()->set_rotationpitch(0);
		npc->_info.mutable_pos()->set_rotationroll(0);
		npc->_info.mutable_pos()->set_rotationyaw(-180.f);

		EnterRoom(npc);
	}
#pragma endregion
}

Zone* Room::GetZone(float targetX, float targetY)
{
	int indexX = abs(_minX - targetX) / _width;
	int indexY = abs(_minY - targetY) / _width;

	return _zones[indexY][indexX];
}

void Room::GetAdjacentZone(float targetX, float targetY, vector<Zone*>& zones)
{
	int indexX = abs(_minX - targetX) / _width;
	int indexY = abs(_minY - targetY) / _width;

	for (int x = indexX - 1; x <= indexX + 1; x++) {
		if (x < 0 || x >= _countX)
			continue;

		for (int y = indexY - 1; y <= indexY + 1; y++) {
			if (y < 0 || y >= _countY)
				continue;

			zones.push_back(_zones[y][x]);
		}
	}
}

float Room::DistanceToTargetSimple(float dx, float dy)
{
	return (pow(abs(dx), 2) + pow(abs(dy), 2));
}

float Room::DistanceToTarget(float dx, float dy)
{
	return sqrt(DistanceToTargetSimple(dx, dy));
}

void Room::Update()
{
	// TODO : ������ �ʿ� �־��
	Execute();
}

void Room::EnterRoom(shared_ptr<GameObject> object)
{
	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(object->_info.objectid());
	PROTOCOL::S_Spawn spawnPkt;

	// �÷��̾�
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// 
		if (_players.count(object->_info.objectid()) == 0) { 
			cout << "PLAYER-" << object->_info.objectid() << " EnterRoom" << endl;

			shared_ptr<Player> player = static_pointer_cast<Player>(object);

			// ��� �÷��̾�� ���� ���� �߰�
			_players[player->_info.objectid()] = player;
			player->_ownerRoom = static_pointer_cast<Room>(shared_from_this());
			
			// �÷��̾� ��(��) �߰�
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->_players.insert(player);
			
			// 
			player->OnEnterGame();

			return;
		}
	}
	// ����
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		 
		if (_monsters.count(object->_info.objectid()) == 0) {
			cout << "MONSTER-" << object->_info.objectid() << " EnterRoom" << endl;

			shared_ptr<Monster> monster = static_pointer_cast<Monster>(object);

			// ��� ���Ϳ��� ���� ���� �߰�
			_monsters[monster->_info.objectid()] = monster;
			monster->_ownerRoom = static_pointer_cast<Room>(shared_from_this());

			// ���� ��(��)�� �߰�
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->_monsters.insert(monster);

			// ���� ��ε� ĳ��Ʈ
			spawnPkt.add_object()->CopyFrom(monster->_info);

			// 
			monster->OnEnterGame();
		}
	}

	else if (type == PROTOCOL::GameObjectType::NPC) {
		if (_npcs.count(object->_info.objectid()) == 0) {
			cout << "NPC-" << object->_info.objectid() << " EnterRoom" << endl;

			shared_ptr<Npc> npc = static_pointer_cast<Npc>(object);

			// ��� npc���� ���� ���� �߰�
			_npcs[npc->_info.objectid()] = npc;
			npc->_ownerRoom = static_pointer_cast<Room>(shared_from_this());

			// npc ��(��)�� �߰�
			Zone* zone = GetZone(npc->_info.pos().locationx(), npc->_info.pos().locationy());
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

void Room::LeaveRoom(int objectId)
{
	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(objectId);
	PROTOCOL::S_DeSpawn despawnPkt;

	// �÷��̾�
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		cout << "PLAYER-" << objectId << " LeaveRoom" << endl;

		auto it = _players.find(objectId);
		if (it != _players.end()) {
			// 
			shared_ptr<Player> player = it->second;
			
			// ��(��)���� �÷��̾� ����
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->Remove(player);

			// �뿡�� �÷��̾� ����
			_players.erase(objectId);

			// �÷��̾� ���� ����(DB)
			player->OnLeaveGame();

			return;
		}
	}
	// ����
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		cout << "MONSTER-" << objectId << " LeaveRoom" << endl;

		auto it = _monsters.find(objectId);
		if (it != _monsters.end()) {
			// 
			shared_ptr<Monster> monster = it->second;

			// ��(��)���� ���� ����
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->Remove(monster);

			// �뿡�� ���� ����
			_monsters.erase(objectId);

			monster->OnLeaveGame();
		}
	}
	// NPC
	else if (type == PROTOCOL::GameObjectType::NPC) {
		cout << "NPC-" << objectId << " LeaveRoom" << endl;

		auto it = _npcs.find(objectId);
		if (it != _npcs.end()) {
			//
			shared_ptr<Npc> npc = it->second;

			// ��(��)���� NPC ����
			Zone* zone = GetZone(npc->_info.pos().locationx(), npc->_info.pos().locationy());
			zone->Remove(npc);

			// �뿡�� npc ����
			_npcs.erase(objectId);

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

void Room::HandleAddItem(shared_ptr<Player> player, PROTOCOL::C_AddItem fromPkt)
{
}

void Room::HandleUseItem(shared_ptr<Player> player, PROTOCOL::C_UseItem fromPkt)
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
	
	// ���� (DELETE)
	if (item->_itemInfo.count() <= 1) {
		itemDB.dbState = DB_STATE::DELETE_NEED;
		DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::DeleteItem, player, itemDB, &Room::DBCallback_UseItem);
	}

	// ���� (UPDATE)
	else {
		itemDB.dbState = DB_STATE::UPDATE_NEED;
		DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateItem, player, itemDB, &Room::DBCallback_UseItem);
	}
}

void Room::HandleEquipItem(shared_ptr<Player> player, PROTOCOL::C_EquipItem fromPkt)
{
	// ������ ��ȸ
	shared_ptr<Item> item = player->_inven.GetItemBySlot(fromPkt.slot());
	if (item == nullptr)
		return;

	// ���������Ѵ�.
	if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE)
		return;

	// ������û�� �Դµ� �������� ������ ����
	if (fromPkt.equip() == true) {
		shared_ptr<Item> equippedItem = player->_inven.FindEquippedSamePos(item);
		if (equippedItem != nullptr) {
			// ���� ��
			if (player->_inven.SetSlotDBLock(equippedItem->_itemInfo.slot(), true) == false) {
				cout << "Room::HandleEquipItem() Error - UnEquip - " << equippedItem->_itemInfo.slot() << "Slot Locked Already" << endl;
				return;
			}

			// DB ��û �غ�
			ItemDB itemDB;
			itemDB.ItemDbId = equippedItem->_itemInfo.itemdbid();
			itemDB.TemplateId = equippedItem->_itemInfo.templateid();
			itemDB.PlayerDbId = equippedItem->_itemInfo.playerdbid();
			itemDB.Count = equippedItem->_itemInfo.count();
			itemDB.Slot = equippedItem->_itemInfo.slot();
			itemDB.Equipped = false;
			itemDB.dbState = DB_STATE::UPDATE_NEED;

			// DB ��û
			DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateItem, player, itemDB, 
				&Room::DBCallback_EquipItem);
		}
	}

	// ���� ��û���� ������ ���� �Ǵ� ����
	// ���� ��
	if (player->_inven.SetSlotDBLock(item->_itemInfo.slot(), true) == false) {
		cout << "Room::HandleEquipItem() Error - Equip - " << item->_itemInfo.slot() << "Slot Locked Already" << endl;
		return;
	}

	// DB ��û �غ�
	ItemDB itemDB;
	itemDB.ItemDbId = item->_itemInfo.itemdbid();
	itemDB.TemplateId = item->_itemInfo.templateid();
	itemDB.PlayerDbId = item->_itemInfo.playerdbid();
	itemDB.Count = item->_itemInfo.count();
	itemDB.Slot = item->_itemInfo.slot();
	itemDB.Equipped = fromPkt.equip();
	itemDB.dbState = DB_STATE::UPDATE_NEED;

	// DB ��û
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateItem, player, itemDB,
		&Room::DBCallback_EquipItem);
}

void Room::HandleAddQuest(shared_ptr<Player> player, PROTOCOL::C_AddQuest fromPkt)
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
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::InsertQuest, player, questDB, &Room::DBCallback_AddQuest);
}

void Room::HandleRemoveQuest(shared_ptr<Player> player, PROTOCOL::C_RemoveQuest fromPkt)
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
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::DeleteQuest, player, questDB, &Room::DBCallback_RemoveQuest);
}

void Room::HandleCompleteQuest(shared_ptr<Player> player, PROTOCOL::C_CompleteQuest fromPkt)
{
	// ����Ʈ ��ȸ -> ����Ʈ �Ϸᰡ�� Ȯ�� -> DB �۾� ��û

	// ����Ʈ ��ȸ

	auto it = player->_questManager->_quests.find(fromPkt.questdbid());
	if (it == player->_questManager->_quests.end()) {
		cout << "Room::HandleCompleteQuest() Error - Can't Find QuestId : " << fromPkt.questdbid() << endl;
		return;
	}

	//shared_ptr<Quest> quest = player->_questManager->_quests[fromPkt.questdbid()]; // QuestDbId�� ǥ�������� QuestTemplateId
	//if (quest == nullptr) {
	//	cout << "Room::HandleCompleteQuest() Error - Can't Find QuestId : " << fromPkt.questdbid() << endl;
	//	return;
	//}

	// ����Ʈ �Ϸᰡ�� Ȯ�� (���൵�� �� ä���°�)
	if (it->second->_questInfo.progress() < it->second->_questData->quantity) {
		cout << "Room::HandleCompleteQuest() Error - progress:" << 
			it->second->_questInfo.progress() << " < quantity:" << it->second->_questData->quantity << endl;
		return;
	}

	// DB ��û �غ�
	QuestDB questDB;
	questDB.QuestDbId = it->second->_questInfo.questdbid();
	questDB.TemplateId = it->second->_questInfo.templateid();
	questDB.PlayerDbId = it->second->_questInfo.playerdbid();
	questDB.Progress = it->second->_questData->quantity;
	questDB.Completed = true;
	questDB.dbState = DB_STATE::UPDATE_NEED;

	// DB ��û
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateQuest, player, questDB, &Room::DBCallback_CompleteQuest);
}

void Room::HandleUpdateQuest(shared_ptr<Player> player, PROTOCOL::C_UpdateQuest fromPkt)
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
	player->_questManager->Update(QuestType::QUEST_TYPE_INTERACT, object->_info.typetemplateid(), fromPkt.questinfo().progress());
}



/*-----------------------------------------------------------------------------
	DB
---------------------------------------------------------------------------*/
void Room::DBCallback_EquipItem(shared_ptr<Player> player, ItemDB itemDB)
{
	// ������Ʈ ���� (DB) -> ������Ʈ -> ��Ŷ -> ���� �� ����
	// ������Ʈ ���� (DB) -> ���Զ� ����

	if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		shared_ptr<Item> item = player->_inven.GetItemBySlot(itemDB.Slot);
		if (item != nullptr) {
			// �κ�(�����޸�)���� ����
			item->_itemInfo.set_equipped(itemDB.Equipped);

			// �÷��̾� �߰����� ����
			player->CalculateAddStat();

			// ��Ŷ
			PROTOCOL::S_EquipItem toPkt;
			toPkt.set_slot(itemDB.Slot);
			toPkt.set_equip(itemDB.Equipped);

			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);
		}
	}

	// ���� �� ����
	player->_inven.SetSlotDBLock(itemDB.Slot, false);
}

void Room::DBCallback_UseItem(shared_ptr<Player> player, ItemDB itemDB)
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
		PROTOCOL::S_UseItem toPkt;
		toPkt.mutable_item()->set_slot(itemDB.Slot);
		toPkt.mutable_item()->set_count(itemDB.Count);
		
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}

	// ����Ʈ ����
	else if (itemDB.dbState == DB_STATE::DELETE_SUCCESS) {
		// �κ��丮���� ������ ����
		player->_inven.Remove(itemDB.Slot);

		// ���� ��Ŷ
		PROTOCOL::S_RemoveItem removePkt;
		removePkt.add_slots(itemDB.Slot);

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(removePkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	
	// ������ ��� ȿ��
	player->UseItem(item->_itemInfo.templateid());

	// ���� �� ����
	player->_inven.SetSlotDBLock(itemDB.Slot, false);
}

void Room::DBCallback_RewardItem(shared_ptr<Player> player, ItemDB itemDB)
{
	// ����, ������Ʈ ���� (DB) -> ������ ����, ������Ʈ (����) -> �κ� �߰� -> ���Զ� ���� -> ��Ŷ
	// ����, ������Ʈ, �� �� ���� (DB) -> ���Զ� ����

	// ����
	if (itemDB.dbState != DB_STATE::INSERT_SUCCESS && itemDB.dbState != DB_STATE::UPDATE_SUCCESS) {
		cout << "Room::RewardItemDBCallback() Error - Result Not INSERT_SUCCESS or Result Not UPDATE_SUCCESS" << endl;
		player->_inven.SetSlotDBLock(itemDB.Slot, false);
		return;
	}

	// ����		
	shared_ptr<Item> item;
	
	// INSERT
	if (itemDB.dbState == DB_STATE::INSERT_SUCCESS) {
		// ������ ����
		item = Item::MakeItem(itemDB);
		if (item == nullptr)
			return;

		// �κ��� ���� �߰�
		player->_inven.Add(item);

		// ��Ŷ
		PROTOCOL::S_AddItem toPkt;
		toPkt.add_items()->CopyFrom(item->_itemInfo);
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	
	// UPDATE
	else if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		// ���� ���� ������ ��ȸ
		item = player->_inven.GetItemBySlot(itemDB.Slot);
		if (item == nullptr) {
			cout << "Room::RewardItemDBCallback() Error - UPDATE_SUCCESS but No Item FInd in Inventory" << endl;
			return;
		}

		// ������ ������Ʈ (�κ��丮�� �ִ�)
		item->_itemInfo.set_itemdbid(itemDB.ItemDbId);
		item->_itemInfo.set_templateid(itemDB.TemplateId);
		item->_itemInfo.set_playerdbid(itemDB.PlayerDbId);
		item->_itemInfo.set_count(itemDB.Count);
		item->_itemInfo.set_slot(itemDB.Slot);
		item->_itemInfo.set_equipped(itemDB.Equipped);

		// ��Ŷ
		PROTOCOL::S_UpdateItem toPkt;
		toPkt.add_items()->CopyFrom(item->_itemInfo);
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	else {
		cout << "Room::RewardItemDBCallback() Error - return else" << endl;
		return;
	}

	// ���Զ� ����
	player->_inven.SetSlotDBLock(item->_itemInfo.slot(), false);
}

// DB - ����Ʈ
void Room::DBCallback_RemoveQuest(shared_ptr<Player> player, QuestDB questDB)
{
	if (questDB.dbState == DB_STATE::DELETE_SUCCESS) {
		if (player->_questManager->Remove(questDB.TemplateId) == true) {
			// ��Ŷ
			PROTOCOL::S_RemoveQuest toPkt;
			toPkt.set_questid(questDB.TemplateId);
			toPkt.set_result(true);
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);
		}
	}
}

void Room::DBCallback_AddQuest(shared_ptr<Player> player, QuestDB questDB)
{
	if (questDB.dbState == DB_STATE::INSERT_SUCCESS) {
		// ����Ʈ ����, �߰�
		shared_ptr<Quest> quest = Quest::MakeQuest(questDB);
		if (player->_questManager->Add(quest) == true) {
			
			// ��Ŷ
			PROTOCOL::S_AddQuest toPkt;
			toPkt.mutable_quest()->set_questdbid(questDB.QuestDbId);
			toPkt.mutable_quest()->set_templateid(questDB.TemplateId);
			toPkt.mutable_quest()->set_playerdbid(questDB.PlayerDbId);
			toPkt.mutable_quest()->set_progress(questDB.Progress);
			toPkt.mutable_quest()->set_completed(questDB.Completed);
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);
		}
	}
}

void Room::DBCallback_CompleteQuest(shared_ptr<Player> player, QuestDB questDB)
{
	// ����Ʈ �Ϸ� �����ؾ��Ѵ�.
	// ���⸦ Ʈ����� �ɾ�� �ϴµ� �ϴ� ���ϴϱ�


	if (questDB.dbState == DB_STATE::UPDATE_SUCCESS) {

		// ����Ʈ ������Ʈ
		if (player->_questManager->Update(questDB) == true) {

			// ��Ŷ
			PROTOCOL::S_CompleteQuest toPkt;
			toPkt.set_questid(questDB.TemplateId);
			toPkt.set_result(true);
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);

			// Exp
			player->AddExp(DataManager::Instance()->_questTable[questDB.TemplateId]->rewardExp);

			// ������ ����
			for (auto p : DataManager::Instance()->_questTable[questDB.TemplateId]->rewardItems) 
				player->TakeReward(p.first, p.second);
		}
	}
}

void Room::DBCallback_UpdateQuest(shared_ptr<Player> player, QuestDB questDB)
{
	// �̰� ����
	if (questDB.dbState == DB_STATE::UPDATE_SUCCESS) {

		// ����Ʈ ������Ʈ
		if (player->_questManager->Update(questDB) == true) {

			// ��Ŷ
			PROTOCOL::S_UpdateQuest toPkt;
			toPkt.mutable_questinfo()->set_questdbid(questDB.QuestDbId);
			toPkt.mutable_questinfo()->set_templateid(questDB.TemplateId);
			toPkt.mutable_questinfo()->set_playerdbid(questDB.PlayerDbId);
			toPkt.mutable_questinfo()->set_progress(questDB.Progress);
			toPkt.mutable_questinfo()->set_completed(questDB.Completed);
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);
		}
	}
}

void Room::Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer)
{
	// ��ε�ĳ��Ʈ�� �� ����
	vector<Zone*> zones;
	GetAdjacentZone(locX, locY, zones);

	// ������ ���� ��� �÷��̾�� ��ε�ĳ����
	for (Zone* z : zones) 
		for (shared_ptr<Player> p : z->_players) 
			p->_ownerSession->SendPacket(sendBuffer);

}

void Room::BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer)
{
	// ��ε�ĳ��Ʈ�� �� ����
	vector<Zone*> zones;
	GetAdjacentZone(player->_info.pos().locationx(), player->_info.pos().locationy(), zones);

	// ������ ������ ���� ���� ��� �÷��̾�� ��ε�ĳ����
	for (Zone* z : zones) {
		for (shared_ptr<Player> p : z->_players) {
			if (player->_info.objectid() == p->_info.objectid())
				continue;

			p->_ownerSession->SendPacket(sendBuffer);
		}
	}
}

void Room::ActorMove(shared_ptr<GameObject> gameObject, PROTOCOL::ObjectInfo info)
{
	// �������� ��ġ ����

	// ����ġ
	float nowX = gameObject->_info.pos().locationx();
	float nowY = gameObject->_info.pos().locationy();
	Zone* nowZone = GetZone(nowX, nowY);

	// �̵��� ��ġ
	float afterX = info.pos().locationx();
	float afterY = info.pos().locationy();
	Zone* afterZone = GetZone(afterX, afterY);

	// ���� ����Ǹ� ��ġ �ű�
	if (nowZone->_zoneId != afterZone->_zoneId) {
		afterZone->Add(gameObject);
		nowZone->Remove(gameObject);
	}

	// ���� ���� ��ġ�� �������� ��ġ ����
	gameObject->_info.mutable_pos()->set_locationx(info.pos().locationx());
	gameObject->_info.mutable_pos()->set_locationy(info.pos().locationy());
	gameObject->_info.mutable_pos()->set_locationz(info.pos().locationz());

	gameObject->_info.mutable_pos()->set_rotationpitch(info.pos().rotationpitch());
	gameObject->_info.mutable_pos()->set_rotationyaw(info.pos().rotationyaw());
	gameObject->_info.mutable_pos()->set_rotationroll(info.pos().rotationroll());
	
	gameObject->_info.mutable_pos()->set_velocityx(info.pos().velocityx());
	gameObject->_info.mutable_pos()->set_velocityy(info.pos().velocityy());
	gameObject->_info.mutable_pos()->set_velocityz(info.pos().velocityz());

	// �̵� ��Ŷ
	PROTOCOL::S_Move toPkt;
	toPkt.mutable_object()->CopyFrom(gameObject->_info);

	// ���� ��ε� ĳ����
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(afterX, afterY, sendBuffer);
}

void Room::ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_Skill fromPkt)
{
	// ��Ÿ�� �� üũ OK - �ϴ� �н�
	
	// ������ ��ġ�� ������� ���� ��Ŷ ��ε�ĳ����
	shared_ptr<GameObject> attacker = ObjectManager::Instance()->Find(fromPkt.attacker());
	if (attacker == nullptr)
		return;

	// ���� ��Ŷ
	PROTOCOL::S_Skill toPkt;
	toPkt.set_attacker(fromPkt.attacker());
	toPkt.set_skillid(fromPkt.skillid());
	for (int victim : fromPkt.victims())
		toPkt.add_victims(victim);
	
	// ���� ��ε�ĳ����
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(attacker->_info.pos().locationx(), attacker->_info.pos().locationy(), sendBuffer);


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

				float dx = gameObject->_info.pos().locationx() - victim->_info.pos().locationx();
				float dy = gameObject->_info.pos().locationy() - victim->_info.pos().locationy();

				// TEST - ��Ÿ� �ȿ� �����, �ϴ� ��Ÿ� 30 ���� ����
				float squaredDT = DistanceToTargetSimple(dx, dy);
				if (squaredDT <= pow(100, 2)) {
					
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

void Room::PlayerChat(shared_ptr<Player> player, PROTOCOL::C_Chat fromPkt)
{
	// ��Ŷ ����
	PROTOCOL::S_Chat toPkt;
	toPkt.mutable_object()->set_name(player->_info.name());
	toPkt.set_text(fromPkt.text());

	// ��ε�ĳ��Ʈ
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(player->_info.pos().locationx(), player->_info.pos().locationy(), sendBuffer);
}

float Room::GetPlayerAround(shared_ptr<Monster> monster)
{
	// ������ ��ġ
	const float& monsterX = monster->_info.pos().locationx();
	const float& monsterY = monster->_info.pos().locationy();

	// ���� Ž�� ���� - (����=�Ÿ�����)
	float squaredNoticeDT = monster->_info.stat().noticedistance();
	squaredNoticeDT = pow(squaredNoticeDT, 2);
	
	// ��ġ�� �ش��ϴ� �ֺ� ��
	vector<Zone*> zones;
	GetAdjacentZone(monsterX, monsterY, zones);

	for (Zone* z : zones) {
		// ������ �÷��̾� ����, 
		for (shared_ptr<Player> p : z->_players) {
			
			// ���Ϳ� �÷��̾� �� �Ÿ�(����)
			float dt = DistanceToTargetSimple(monsterX - p->_info.pos().locationx(), monsterY - p->_info.pos().locationy());

			// ���� Ž�� ����(����)�� �����÷��̾�Ÿ�(����) ��
			if (dt <= squaredNoticeDT) {
				// ���� Ÿ�� ����
				monster->_target = p;

				// ������ ��ȯ
				return sqrt(dt);
			}
		}
	}
	
	// �÷��̾� ������ ������ ����
	return -1.f;
}

