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

// 일단 정수에서 나누어 떨어지게 맵의 크기를 조정할 것임
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
	// 나중에 수정
	{// 몬스터
		// 0번 몬스터 생성
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// 베이스 위치, 위치
		monster->_basePos._x = 2000;
		monster->_basePos._y = -3000;
		monster->_basePos._z = 145.f;
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		// 몬스터 게임룸 진입
		EnterRoom(monster);
	}
	{// 몬스터
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
	{// 몬스터
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
	{// 몬스터
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
	// TODO : 제거할 필요 있어보임
	Execute();
}

void Room::EnterRoom(shared_ptr<GameObject> object)
{
	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(object->_info.objectid());
	PROTOCOL::S_Spawn spawnPkt;

	// 플레이어
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// 
		if (_players.count(object->_info.objectid()) == 0) { 
			cout << "PLAYER-" << object->_info.objectid() << " EnterRoom" << endl;

			shared_ptr<Player> player = static_pointer_cast<Player>(object);

			// 룸과 플레이어에서 서로 참조 추가
			_players[player->_info.objectid()] = player;
			player->_ownerRoom = static_pointer_cast<Room>(shared_from_this());
			
			// 플레이어 맵(존) 추가
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->_players.insert(player);
			
			// 
			player->OnEnterGame();

			return;
		}
	}
	// 몬스터
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		 
		if (_monsters.count(object->_info.objectid()) == 0) {
			cout << "MONSTER-" << object->_info.objectid() << " EnterRoom" << endl;

			shared_ptr<Monster> monster = static_pointer_cast<Monster>(object);

			// 룸과 몬스터에서 서로 참조 추가
			_monsters[monster->_info.objectid()] = monster;
			monster->_ownerRoom = static_pointer_cast<Room>(shared_from_this());

			// 몬스터 맵(존)에 추가
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->_monsters.insert(monster);

			// 스폰 브로드 캐스트
			spawnPkt.add_object()->CopyFrom(monster->_info);

			// 
			monster->OnEnterGame();
		}
	}

	else if (type == PROTOCOL::GameObjectType::NPC) {
		if (_npcs.count(object->_info.objectid()) == 0) {
			cout << "NPC-" << object->_info.objectid() << " EnterRoom" << endl;

			shared_ptr<Npc> npc = static_pointer_cast<Npc>(object);

			// 룸과 npc에서 서로 참조 추가
			_npcs[npc->_info.objectid()] = npc;
			npc->_ownerRoom = static_pointer_cast<Room>(shared_from_this());

			// npc 맵(존)에 추가
			Zone* zone = GetZone(npc->_info.pos().locationx(), npc->_info.pos().locationy());
			zone->_npcs.insert((npc));

			// 스폰 브로드 캐스트
			spawnPkt.add_object()->CopyFrom(npc->_info);

			//
			npc->OnEnterGame();
		}
	}

	// 투사체, 기타등등...
	else {
		return;
	}
}

void Room::LeaveRoom(int objectId)
{
	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(objectId);
	PROTOCOL::S_DeSpawn despawnPkt;

	// 플레이어
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		cout << "PLAYER-" << objectId << " LeaveRoom" << endl;

		auto it = _players.find(objectId);
		if (it != _players.end()) {
			// 
			shared_ptr<Player> player = it->second;
			
			// 맵(존)에서 플레이어 제거
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->Remove(player);

			// 룸에서 플레이어 제거
			_players.erase(objectId);

			// 플레이어 스탯 저장(DB)
			player->OnLeaveGame();

			return;
		}
	}
	// 몬스터
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		cout << "MONSTER-" << objectId << " LeaveRoom" << endl;

		auto it = _monsters.find(objectId);
		if (it != _monsters.end()) {
			// 
			shared_ptr<Monster> monster = it->second;

			// 맵(존)에서 몬스터 제거
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->Remove(monster);

			// 룸에서 몬스터 제거
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

			// 맵(존)에서 NPC 제거
			Zone* zone = GetZone(npc->_info.pos().locationx(), npc->_info.pos().locationy());
			zone->Remove(npc);

			// 룸에서 npc 제거
			_npcs.erase(objectId);

			//
			npc->OnLeaveGame();
		}
	}
	// 투사체, 기타등등
	else {
		return;
	}
}

/*-----------------------------------------------------------------------------
	패킷 핸들
---------------------------------------------------------------------------*/

void Room::HandleAddItem(shared_ptr<Player> player, PROTOCOL::C_AddItem fromPkt)
{
}

void Room::HandleUseItem(shared_ptr<Player> player, PROTOCOL::C_UseItem fromPkt)
{
	// 아이템 조회
	shared_ptr<Item> item = player->_inven.GetItemBySlot(fromPkt.slot());
	if (item == nullptr)
		return;

	// 소비류여야한다.
	if (item->_itemType != PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE)
		return;

	// 슬롯 락
	if (player->_inven.SetSlotDBLock(fromPkt.slot(), true) == false) {
		cout << "Room::HandleUseItem() Error - " << fromPkt.slot() << "Slot Locked Already" << endl;
		return;
	}
		
	// DB 요청 준비
	ItemDB itemDB;
	itemDB.ItemDbId = item->_itemInfo.itemdbid();
	itemDB.TemplateId = item->_itemInfo.templateid();
	itemDB.PlayerDbId = item->_itemInfo.playerdbid();
	itemDB.Count = item->_itemInfo.count() - 1;
	itemDB.Slot = item->_itemInfo.slot();
	itemDB.Equipped = item->_itemInfo.equipped();

	// 소비 후 유지가 안되면 => 1개남았는데 사용 -> 삭제
	// 소비 후 유지가 되면 => 100개 남았는데 사용 -> 99개 -> 유지
	
	// 삭제 (DELETE)
	if (item->_itemInfo.count() <= 1) {
		itemDB.dbState = DB_STATE::DELETE_NEED;
		DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::DeleteItem, player, itemDB, &Room::DBCallback_UseItem);
	}

	// 유지 (UPDATE)
	else {
		itemDB.dbState = DB_STATE::UPDATE_NEED;
		DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateItem, player, itemDB, &Room::DBCallback_UseItem);
	}
}

void Room::HandleEquipItem(shared_ptr<Player> player, PROTOCOL::C_EquipItem fromPkt)
{
	// 아이템 조회
	shared_ptr<Item> item = player->_inven.GetItemBySlot(fromPkt.slot());
	if (item == nullptr)
		return;

	// 장비류여야한다.
	if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE)
		return;

	// 장착요청이 왔는데 선장착이 있으면 해제
	if (fromPkt.equip() == true) {
		shared_ptr<Item> equippedItem = player->_inven.FindEquippedSamePos(item);
		if (equippedItem != nullptr) {
			// 슬롯 락
			if (player->_inven.SetSlotDBLock(equippedItem->_itemInfo.slot(), true) == false) {
				cout << "Room::HandleEquipItem() Error - UnEquip - " << equippedItem->_itemInfo.slot() << "Slot Locked Already" << endl;
				return;
			}

			// DB 요청 준비
			ItemDB itemDB;
			itemDB.ItemDbId = equippedItem->_itemInfo.itemdbid();
			itemDB.TemplateId = equippedItem->_itemInfo.templateid();
			itemDB.PlayerDbId = equippedItem->_itemInfo.playerdbid();
			itemDB.Count = equippedItem->_itemInfo.count();
			itemDB.Slot = equippedItem->_itemInfo.slot();
			itemDB.Equipped = false;
			itemDB.dbState = DB_STATE::UPDATE_NEED;

			// DB 요청
			DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateItem, player, itemDB, 
				&Room::DBCallback_EquipItem);
		}
	}

	// 원래 요청들어온 아이템 장착 또는 해제
	// 슬롯 락
	if (player->_inven.SetSlotDBLock(item->_itemInfo.slot(), true) == false) {
		cout << "Room::HandleEquipItem() Error - Equip - " << item->_itemInfo.slot() << "Slot Locked Already" << endl;
		return;
	}

	// DB 요청 준비
	ItemDB itemDB;
	itemDB.ItemDbId = item->_itemInfo.itemdbid();
	itemDB.TemplateId = item->_itemInfo.templateid();
	itemDB.PlayerDbId = item->_itemInfo.playerdbid();
	itemDB.Count = item->_itemInfo.count();
	itemDB.Slot = item->_itemInfo.slot();
	itemDB.Equipped = fromPkt.equip();
	itemDB.dbState = DB_STATE::UPDATE_NEED;

	// DB 요청
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateItem, player, itemDB,
		&Room::DBCallback_EquipItem);
}

void Room::HandleAddQuest(shared_ptr<Player> player, PROTOCOL::C_AddQuest fromPkt)
{
	// 해당 란에 이미 퀘스트가 있으면 패스
	// 여기서 시간 많이 잡아먹었다. 맵컨테이너에서 []로 접근할 때 nullptr로 값을 생성
	// shared_ptr<Quest> quest = player->_questManager->_quests[fromPkt.quest().templateid()];
	auto it = player->_questManager->_quests.find(fromPkt.quest().templateid());
	if (it != player->_questManager->_quests.end()) {
		cout << "Room::HandleAddQuest() Error - QuestTemplateId:" << fromPkt.quest().templateid() << " is Already Exists" << endl;
		return;
	}

	// DB 요청 준비
	QuestDB questDB;
	questDB.TemplateId = fromPkt.quest().templateid();
	questDB.PlayerDbId = player->_info.playerdbid();
	questDB.Progress = 0;
	questDB.Completed = false;
	questDB.dbState = DB_STATE::INSERT_NEED;

	// DB 요청
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::InsertQuest, player, questDB, &Room::DBCallback_AddQuest);
}

void Room::HandleRemoveQuest(shared_ptr<Player> player, PROTOCOL::C_RemoveQuest fromPkt)
{
	// 퀘스트 조회
	shared_ptr<Quest> quest = player->_questManager->_quests[fromPkt.questdbid()]; 
	if (quest == nullptr) {
		cout << "Room::HandleRemoveQuest() Error - Can't Find QuestId: " << fromPkt.questdbid() << endl;
		return;
	}
	
	// DB 요청 준비
	QuestDB questDB;
	questDB.QuestDbId = quest->_questInfo.questdbid();
	questDB.TemplateId = quest->_questInfo.templateid();
	questDB.PlayerDbId = quest->_questInfo.playerdbid();
	questDB.Progress = quest->_questInfo.progress();
	questDB.Completed = quest->_questInfo.completed();
	questDB.dbState = DB_STATE::DELETE_NEED;

	// DB 요청
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::DeleteQuest, player, questDB, &Room::DBCallback_RemoveQuest);
}

void Room::HandleCompleteQuest(shared_ptr<Player> player, PROTOCOL::C_CompleteQuest fromPkt)
{
	// 퀘스트 조회 -> 퀘스트 완료가능 확인 -> DB 작업 요청

	// 퀘스트 조회

	auto it = player->_questManager->_quests.find(fromPkt.questdbid());
	if (it == player->_questManager->_quests.end()) {
		cout << "Room::HandleCompleteQuest() Error - Can't Find QuestId : " << fromPkt.questdbid() << endl;
		return;
	}

	//shared_ptr<Quest> quest = player->_questManager->_quests[fromPkt.questdbid()]; // QuestDbId라 표기하지만 QuestTemplateId
	//if (quest == nullptr) {
	//	cout << "Room::HandleCompleteQuest() Error - Can't Find QuestId : " << fromPkt.questdbid() << endl;
	//	return;
	//}

	// 퀘스트 완료가능 확인 (진행도를 다 채웠는가)
	if (it->second->_questInfo.progress() < it->second->_questData->quantity) {
		cout << "Room::HandleCompleteQuest() Error - progress:" << 
			it->second->_questInfo.progress() << " < quantity:" << it->second->_questData->quantity << endl;
		return;
	}

	// DB 요청 준비
	QuestDB questDB;
	questDB.QuestDbId = it->second->_questInfo.questdbid();
	questDB.TemplateId = it->second->_questInfo.templateid();
	questDB.PlayerDbId = it->second->_questInfo.playerdbid();
	questDB.Progress = it->second->_questData->quantity;
	questDB.Completed = true;
	questDB.dbState = DB_STATE::UPDATE_NEED;

	// DB 요청
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateQuest, player, questDB, &Room::DBCallback_CompleteQuest);
}

void Room::HandleUpdateQuest(shared_ptr<Player> player, PROTOCOL::C_UpdateQuest fromPkt)
{
	// 상호 작용 업데이트
	
	// 게임오브젝트 조회
	shared_ptr<GameObject> object  = static_pointer_cast<Npc>(ObjectManager::Instance()->_objects[fromPkt.objectid()]);
	if (object == nullptr)
		return;

	// NPC가 아니면 패스
	if (object->_info.objecttype() != PROTOCOL::GameObjectType::NPC)
		return;

	// 거리 확인 및 검증 - 일단 패스

	// 업데이트
	player->_questManager->Update(QuestType::QUEST_TYPE_INTERACT, object->_info.typetemplateid(), fromPkt.questinfo().progress());
}



/*-----------------------------------------------------------------------------
	DB
---------------------------------------------------------------------------*/
void Room::DBCallback_EquipItem(shared_ptr<Player> player, ItemDB itemDB)
{
	// 업데이트 성공 (DB) -> 업데이트 -> 패킷 -> 슬롯 락 해제
	// 업데이트 실패 (DB) -> 슬롯락 해제

	if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		shared_ptr<Item> item = player->_inven.GetItemBySlot(itemDB.Slot);
		if (item != nullptr) {
			// 인벤(서버메모리)에서 장착
			item->_itemInfo.set_equipped(itemDB.Equipped);

			// 플레이어 추가스탯 재계산
			player->CalculateAddStat();

			// 패킷
			PROTOCOL::S_EquipItem toPkt;
			toPkt.set_slot(itemDB.Slot);
			toPkt.set_equip(itemDB.Equipped);

			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);
		}
	}

	// 슬롯 락 해제
	player->_inven.SetSlotDBLock(itemDB.Slot, false);
}

void Room::DBCallback_UseItem(shared_ptr<Player> player, ItemDB itemDB)
{
	// 삭제, 업데이트 성공 (DB) -> 삭제, 업데이트 (서버) -> 슬롯락 해제 -> 아이템 효과 -> 패킷
	// 삭제, 업데이트, 그 외 실패 (DB) -> 슬롯락 해제
	
	// 실패
	if (itemDB.dbState != DB_STATE::UPDATE_SUCCESS && itemDB.dbState != DB_STATE::DELETE_SUCCESS) {
		cout << "Room::DBCallback_UseItem() Error - dbState Not UPDATE_SUCCESS & dbState Not DELETE_SUCCESS" << endl;
		player->_inven.SetSlotDBLock(itemDB.Slot, false);
		return;
	}

	// 성공
	// 아이템 조회
	shared_ptr<Item> item = player->_inven.GetItemBySlot(itemDB.Slot);
	if (item == nullptr) {
		cout << "Room::DBCallback_UseItem() Error - Can't Find ItemSlot:" << itemDB.Slot << endl;
		return;
	}
		
	// 업데이트 성공
	if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		// 인벤토리에서 아이템 업데이트
		item->_itemInfo.set_count(itemDB.Count);

		// 사용 패킷
		PROTOCOL::S_UseItem toPkt;
		toPkt.mutable_item()->set_slot(itemDB.Slot);
		toPkt.mutable_item()->set_count(itemDB.Count);
		
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}

	// 딜리트 성공
	else if (itemDB.dbState == DB_STATE::DELETE_SUCCESS) {
		// 인벤토리에서 아이템 제거
		player->_inven.Remove(itemDB.Slot);

		// 제거 패킷
		PROTOCOL::S_RemoveItem removePkt;
		removePkt.add_slots(itemDB.Slot);

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(removePkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	
	// 아이템 사용 효과
	player->UseItem(item->_itemInfo.templateid());

	// 슬롯 락 해제
	player->_inven.SetSlotDBLock(itemDB.Slot, false);
}

void Room::DBCallback_RewardItem(shared_ptr<Player> player, ItemDB itemDB)
{
	// 생성, 업데이트 성공 (DB) -> 아이템 생성, 업데이트 (서버) -> 인벤 추가 -> 슬롯락 해제 -> 패킷
	// 생성, 업데이트, 그 외 실패 (DB) -> 슬롯락 해제

	// 실패
	if (itemDB.dbState != DB_STATE::INSERT_SUCCESS && itemDB.dbState != DB_STATE::UPDATE_SUCCESS) {
		cout << "Room::RewardItemDBCallback() Error - Result Not INSERT_SUCCESS or Result Not UPDATE_SUCCESS" << endl;
		player->_inven.SetSlotDBLock(itemDB.Slot, false);
		return;
	}

	// 성공		
	shared_ptr<Item> item;
	
	// INSERT
	if (itemDB.dbState == DB_STATE::INSERT_SUCCESS) {
		// 아이템 생성
		item = Item::MakeItem(itemDB);
		if (item == nullptr)
			return;

		// 인벤에 새로 추가
		player->_inven.Add(item);

		// 패킷
		PROTOCOL::S_AddItem toPkt;
		toPkt.add_items()->CopyFrom(item->_itemInfo);
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	
	// UPDATE
	else if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		// 기존 슬롯 아이템 조회
		item = player->_inven.GetItemBySlot(itemDB.Slot);
		if (item == nullptr) {
			cout << "Room::RewardItemDBCallback() Error - UPDATE_SUCCESS but No Item FInd in Inventory" << endl;
			return;
		}

		// 아이템 업데이트 (인벤토리에 있는)
		item->_itemInfo.set_itemdbid(itemDB.ItemDbId);
		item->_itemInfo.set_templateid(itemDB.TemplateId);
		item->_itemInfo.set_playerdbid(itemDB.PlayerDbId);
		item->_itemInfo.set_count(itemDB.Count);
		item->_itemInfo.set_slot(itemDB.Slot);
		item->_itemInfo.set_equipped(itemDB.Equipped);

		// 패킷
		PROTOCOL::S_UpdateItem toPkt;
		toPkt.add_items()->CopyFrom(item->_itemInfo);
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	else {
		cout << "Room::RewardItemDBCallback() Error - return else" << endl;
		return;
	}

	// 슬롯락 해제
	player->_inven.SetSlotDBLock(item->_itemInfo.slot(), false);
}

// DB - 퀘스트
void Room::DBCallback_RemoveQuest(shared_ptr<Player> player, QuestDB questDB)
{
	if (questDB.dbState == DB_STATE::DELETE_SUCCESS) {
		if (player->_questManager->Remove(questDB.TemplateId) == true) {
			// 패킷
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
		// 퀘스트 생성, 추가
		shared_ptr<Quest> quest = Quest::MakeQuest(questDB);
		if (player->_questManager->Add(quest) == true) {
			
			// 패킷
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
	// 퀘스트 완료 수정해야한다.
	// 여기를 트랜잭션 걸어야 하는데 일단 급하니까


	if (questDB.dbState == DB_STATE::UPDATE_SUCCESS) {

		// 퀘스트 업데이트
		if (player->_questManager->Update(questDB) == true) {

			// 패킷
			PROTOCOL::S_CompleteQuest toPkt;
			toPkt.set_questid(questDB.TemplateId);
			toPkt.set_result(true);
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);

			// Exp
			player->AddExp(DataManager::Instance()->_questTable[questDB.TemplateId]->rewardExp);

			// 아이템 보상
			for (auto p : DataManager::Instance()->_questTable[questDB.TemplateId]->rewardItems) 
				player->TakeReward(p.first, p.second);
		}
	}
}

void Room::DBCallback_UpdateQuest(shared_ptr<Player> player, QuestDB questDB)
{
	// 이거 ㄴㄴ
	if (questDB.dbState == DB_STATE::UPDATE_SUCCESS) {

		// 퀘스트 업데이트
		if (player->_questManager->Update(questDB) == true) {

			// 패킷
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
	// 브로드캐스트할 존 추출
	vector<Zone*> zones;
	GetAdjacentZone(locX, locY, zones);

	// 추출한 존의 모든 플레이어에게 브로드캐스팅
	for (Zone* z : zones) 
		for (shared_ptr<Player> p : z->_players) 
			p->_ownerSession->SendPacket(sendBuffer);

}

void Room::BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer)
{
	// 브로드캐스트할 존 추출
	vector<Zone*> zones;
	GetAdjacentZone(player->_info.pos().locationx(), player->_info.pos().locationy(), zones);

	// 추출한 존에서 본인 제외 모든 플레이어에게 브로드캐스팅
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
	// 서버에서 위치 변경

	// 현위치
	float nowX = gameObject->_info.pos().locationx();
	float nowY = gameObject->_info.pos().locationy();
	Zone* nowZone = GetZone(nowX, nowY);

	// 이동할 위치
	float afterX = info.pos().locationx();
	float afterY = info.pos().locationy();
	Zone* afterZone = GetZone(afterX, afterY);

	// 존이 변경되면 위치 옮김
	if (nowZone->_zoneId != afterZone->_zoneId) {
		afterZone->Add(gameObject);
		nowZone->Remove(gameObject);
	}

	// 보고 받은 위치로 서버에서 위치 갱신
	gameObject->_info.mutable_pos()->set_locationx(info.pos().locationx());
	gameObject->_info.mutable_pos()->set_locationy(info.pos().locationy());
	gameObject->_info.mutable_pos()->set_locationz(info.pos().locationz());

	gameObject->_info.mutable_pos()->set_rotationpitch(info.pos().rotationpitch());
	gameObject->_info.mutable_pos()->set_rotationyaw(info.pos().rotationyaw());
	gameObject->_info.mutable_pos()->set_rotationroll(info.pos().rotationroll());
	
	gameObject->_info.mutable_pos()->set_velocityx(info.pos().velocityx());
	gameObject->_info.mutable_pos()->set_velocityy(info.pos().velocityy());
	gameObject->_info.mutable_pos()->set_velocityz(info.pos().velocityz());

	// 이동 패킷
	PROTOCOL::S_Move toPkt;
	toPkt.mutable_object()->CopyFrom(gameObject->_info);

	// 지역 브로드 캐스팅
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(afterX, afterY, sendBuffer);
}

void Room::ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_Skill fromPkt)
{
	// 쿨타임 등 체크 OK - 일단 패스
	
	// 공격자 위치를 기반으로 공격 패킷 브로드캐스팅
	shared_ptr<GameObject> attacker = ObjectManager::Instance()->Find(fromPkt.attacker());
	if (attacker == nullptr)
		return;

	// 공격 패킷
	PROTOCOL::S_Skill toPkt;
	toPkt.set_attacker(fromPkt.attacker());
	toPkt.set_skillid(fromPkt.skillid());
	for (int victim : fromPkt.victims())
		toPkt.add_victims(victim);
	
	// 지역 브로드캐스팅
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(attacker->_info.pos().locationx(), attacker->_info.pos().locationy(), sendBuffer);


	// 스킬 판정
	// 스킬 정보 확인
	auto it = DataManager::Instance()->_skillTable.find(fromPkt.skillid());
	if (it != DataManager::Instance()->_skillTable.end()) {
		// 스킬 정보
		SkillData skillData = it->second;
		
		// 스킬 종류에 따라 처리
		switch (skillData.skillType) {
		// 평타
		case PROTOCOL::SkillType::SKILL_AUTO: 
		{
			// 희생자들
			for (int i = 0; i < fromPkt.victims_size(); i++) {
				// 
				shared_ptr<GameObject> victim = ObjectManager::Instance()->Find(fromPkt.victims(i));
				if (victim == nullptr) 
					continue;

				float dx = gameObject->_info.pos().locationx() - victim->_info.pos().locationx();
				float dy = gameObject->_info.pos().locationy() - victim->_info.pos().locationy();

				// TEST - 사거리 안에 드는지, 일단 사거리 30 임의 지정
				float squaredDT = DistanceToTargetSimple(dx, dy);
				if (squaredDT <= pow(100, 2)) {
					
					// 총공격력 + 스킬 대미지
					// 플레이어 = 아이템 대미지 + 스텟 대미지 + 스킬 대미지
					// 몬스터 = 스텟 대미지 + 스킬 대미지
					victim->OnDamaged(attacker, attacker->_totalDamage + skillData.damage);
				}
			}
		} 
		break;

		// 그 외
		default:
			break;
		}
	}
}

void Room::PlayerChat(shared_ptr<Player> player, PROTOCOL::C_Chat fromPkt)
{
	// 패킷 생성
	PROTOCOL::S_Chat toPkt;
	toPkt.mutable_object()->set_name(player->_info.name());
	toPkt.set_text(fromPkt.text());

	// 브로드캐스트
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(player->_info.pos().locationx(), player->_info.pos().locationy(), sendBuffer);
}

float Room::GetPlayerAround(shared_ptr<Monster> monster)
{
	// 몬스터의 위치
	const float& monsterX = monster->_info.pos().locationx();
	const float& monsterY = monster->_info.pos().locationy();

	// 몬스터 탐색 범위 - (간략=거리제곱)
	float squaredNoticeDT = monster->_info.stat().noticedistance();
	squaredNoticeDT = pow(squaredNoticeDT, 2);
	
	// 위치에 해당하는 주변 존
	vector<Zone*> zones;
	GetAdjacentZone(monsterX, monsterY, zones);

	for (Zone* z : zones) {
		// 존에서 플레이어 색출, 
		for (shared_ptr<Player> p : z->_players) {
			
			// 몬스터와 플레이어 간 거리(간략)
			float dt = DistanceToTargetSimple(monsterX - p->_info.pos().locationx(), monsterY - p->_info.pos().locationy());

			// 몬스터 탐색 범위(간략)과 몬스터플레이어간거리(간략) 비교
			if (dt <= squaredNoticeDT) {
				// 몬스터 타겟 설정
				monster->_target = p;

				// 실제값 반환
				return sqrt(dt);
			}
		}
	}
	
	// 플레이어 없으면 음수값 리턴
	return -1.f;
}

