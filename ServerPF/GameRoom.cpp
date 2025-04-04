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

// 일단 정수에서 나누어 떨어지게 맵의 크기를 조정할 것임
void GameRoom::Init()
{
	// TODO : Config
	_gameMap->Load();

	
#pragma region SpawnMonsterAndNPC

	// 나중에 수정
	{// 몬스터
		// 0번 몬스터 생성
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// 베이스 위치, 위치
		monster->_basePos.CopyFrom(MakePFVector(2000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		// 몬스터 게임룸 진입
		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(2000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(4000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(4000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(6000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(6000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);

		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos.CopyFrom(MakePFVector(8000, 0, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);
		
		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos.CopyFrom(MakePFVector(8000, -3000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);
		
		EnterRoom(monster);
	}
	{// 몬스터
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		monster->_basePos.CopyFrom(MakePFVector(0, -5000, 145.f));
		monster->_info.mutable_pos()->mutable_location()->CopyFrom(monster->_basePos);
		
		EnterRoom(monster);
	}
	{// 몬스터
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

	// 플레이어
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// 
		if (_gameMap->_players.count(object->_info.objectid()) == 0) {
			// 플레이어로 형변환
			shared_ptr<Player> player = static_pointer_cast<Player>(object);

			// cout << "[Room] SESSION-" << player->_ownerSession->_sessionId << ", PLAYER-" << object->_info.objectid() << " EnterRoom" << endl;
			
			// 룸과 플레이어에서 서로 참조 추가
			_gameMap->_players[player->_info.objectid()] = player;
			player->_ownerRoom = static_pointer_cast<GameRoom>(shared_from_this());

			// 플레이어 맵(존) 추가
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
	// 몬스터
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		 
		if (_gameMap->_monsters.count(object->_info.objectid()) == 0) {
			cout << "[Room] MONSTER-" << object->_info.objectid() << " EnterRoom" << endl;

			// 몬스터로 형변환
			shared_ptr<Monster> monster = static_pointer_cast<Monster>(object);

			// 룸과 몬스터에서 서로 참조 추가
			_gameMap->_monsters[monster->_info.objectid()] = monster;
			monster->_ownerRoom = static_pointer_cast<GameRoom>(shared_from_this());

			// 몬스터 맵(존)에 추가
			shared_ptr<Zone> zone = GetZone(monster->_info.pos().location().x(), monster->_info.pos().location().y());
			zone->_monsters.insert(monster);

			// 스폰 브로드 캐스트
			spawnPkt.add_object()->CopyFrom(monster->_info);

			// 
			monster->OnEnterGame();
		}
	}

	else if (type == PROTOCOL::GameObjectType::NPC) {
		if (_gameMap->_npcs.count(object->_info.objectid()) == 0) {
			cout << "[Room] NPC-" << object->_info.objectid() << " EnterRoom" << endl;

			// NPC로 형변환
			shared_ptr<Npc> npc = static_pointer_cast<Npc>(object);

			// 룸과 npc에서 서로 참조 추가
			_gameMap->_npcs[npc->_info.objectid()] = npc;
			npc->_ownerRoom = static_pointer_cast<GameRoom>(shared_from_this());

			// npc 맵(존)에 추가
			shared_ptr<Zone> zone = GetZone(npc->_info.pos().location().x(), npc->_info.pos().location().y());
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

void GameRoom::LeaveRoom(int objectId)
{
	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(objectId);
	PROTOCOL::S_DESPAWN despawnPkt;

	// 플레이어
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// cout << "[Room] PLAYER-" << objectId << " LeaveRoom" << endl;

		auto it = _gameMap->_players.find(objectId);
		if (it != _gameMap->_players.end()) {
			// 
			shared_ptr<Player> player = it->second;
			
			cout << "[Room] PLAYER-" << objectId << " LeaveRoom" << endl;

			// 맵(존)에서 플레이어 제거
			shared_ptr<Zone>& zone = GetZone(player->_info.pos().location().x(), player->_info.pos().location().y());
			zone->Remove(player);

			// 룸에서 플레이어 제거
			if (_gameMap->_players.erase(objectId) == 0) {
				// cout << "Room::LeaveRoom() Erase Player From _players Couldn't Error" << endl;
			}

			// 플레이어 스탯 저장(DB)
			player->OnLeaveGame();

			// 오브젝트 매니저에서 플레이어 제거
			ObjectManager::Instance()->Remove(player->_info.objectid());

			// 세션에서 플레이어 제거 - 문제 유발할 수 잇음 계속 확인 필요
			player->_ownerSession->_player.reset();

			return;
		}
	}
	// 몬스터
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		cout << "[Room] MONSTER-" << objectId << " LeaveRoom" << endl;

		auto it = _gameMap->_monsters.find(objectId);
		if (it != _gameMap->_monsters.end()) {
			// 
			shared_ptr<Monster> monster = it->second;

			// 맵(존)에서 몬스터 제거
			shared_ptr<Zone>& zone = GetZone(monster->_info.pos().location().x(), monster->_info.pos().location().y());
			zone->Remove(monster);

			// 룸에서 몬스터 제거
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

			// 맵(존)에서 NPC 제거
			shared_ptr<Zone>& zone = GetZone(npc->_info.pos().location().x(), npc->_info.pos().location().y());
			zone->Remove(npc);

			// 룸에서 npc 제거
			_gameMap->_npcs.erase(objectId);

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


void GameRoom::HandleUseItem(shared_ptr<Player> player, PROTOCOL::C_USE_ITEM fromPkt)
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
	
	// 삭제 (DELETE) or 유지 (UPDATE)
	if (item->_itemInfo.count() <= 1)
		itemDB.dbState = DB_STATE::DELETE_NEED;
	else 
		itemDB.dbState = DB_STATE::UPDATE_NEED;

	// DB 요청
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_UseItem, player, itemDB);
}

void GameRoom::HandleEquipItem(shared_ptr<Player> player, PROTOCOL::C_EQUIP_ITEM fromPkt)
{
	// 아이템 조회
	shared_ptr<Item> item = player->_inven.GetItemBySlot(fromPkt.slot());
	if (item == nullptr)
		return;

	// 장비류여야한다.
	if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE)
		return;

	// 해제할 장비(요청이 장착 요청 && 해당 부위에 이미 장착된 장비가 있으면)
	if (fromPkt.equip() == true) {
		shared_ptr<Item> equippedItem = player->_inven.FindEquippedSamePos(item);
		if (equippedItem != nullptr) {
			// 슬롯 락
			if (player->_inven.SetSlotDBLock(equippedItem->_itemInfo.slot(), true) == false) {
				cout << "Room::HandleEquipItem() Error - Equipment to UnEquip, Slot:" << equippedItem->_itemInfo.slot() << ", SlotLocked Already" << endl;
				return;
			}

			// 서버 메모리 장비 해제
			equippedItem->_itemInfo.set_equipped(false);

			// DB 노티
			DBManager::Instance()->_gameDbManager->TransactNoti_EquipItem(player, equippedItem);

			// 패킷
			PROTOCOL::S_EQUIP_ITEM toPkt;
			toPkt.set_equip(equippedItem->_itemInfo.equipped());
			toPkt.set_slot(equippedItem->_itemInfo.slot());
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
			player->_ownerSession->SendPacket(sendBuffer);

			// 슬롯 락 해제
			player->_inven.SetSlotDBLock(equippedItem->_itemInfo.slot(), false);
		}
	}

	// 요청 받은 장착 또는 장착 해제
	// 슬롯 락
	if (player->_inven.SetSlotDBLock(item->_itemInfo.slot(), true) == false) {
		cout << "Room::HandleEquipItem() Error - Equipment to Equip, Slot:" << item->_itemInfo.slot() << ", SlotLocked Already" << endl;
		return;
	}

	// 서버 메모리 장비 해제
	item->_itemInfo.set_equipped(fromPkt.equip());

	// DB 노티
	DBManager::Instance()->_gameDbManager->TransactNoti_EquipItem(player, item);

	// 패킷
	PROTOCOL::S_EQUIP_ITEM toPkt;
	toPkt.set_equip(item->_itemInfo.equipped());
	toPkt.set_slot(item->_itemInfo.slot());
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	player->_ownerSession->SendPacket(sendBuffer);

	// 슬롯 락 해제
	player->_inven.SetSlotDBLock(item->_itemInfo.slot(), false);
}

void GameRoom::HandleAddQuest(shared_ptr<Player> player, PROTOCOL::C_ADD_QUEST fromPkt)
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
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_AddQuest, player, questDB);
}

void GameRoom::HandleRemoveQuest(shared_ptr<Player> player, PROTOCOL::C_REMOVE_QUEST fromPkt)
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
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_RemoveQuest, player, questDB);
}

void GameRoom::HandleCompleteQuest(shared_ptr<Player> player, PROTOCOL::C_COMPLETE_QUEST fromPkt)
{
	// 퀘스트 조회
	auto it = player->_questManager->_quests.find(fromPkt.questdbid());
	if (it == player->_questManager->_quests.end())
		return;

	// 퀘스트 완료 가능 확인
	if (it->second->_questInfo.progress() < it->second->_questData->quantity)
		return;

	// 퀘스트 보상 아이템 목록 정보
	vector<ItemDB> itemDBs;
	for (auto p : it->second->_questData->rewardItems) {
		ItemDB itemDB = player->CanAddItemDB(p.first, p.second);
		if (itemDB.Slot == -1) {
			// 슬롯 락 해제
			for (ItemDB itemdb : itemDBs)
				player->_inven.SetSlotDBLock(itemdb.Slot, false);

			return;
		}

		itemDBs.push_back(itemDB);
	}

	// 퀘스트 정보
	QuestDB questDB;
	questDB.QuestDbId = it->second->_questInfo.questdbid();
	questDB.TemplateId = it->second->_questInfo.templateid();
	questDB.Progress = it->second->_questInfo.progress();
	questDB.Completed = true;
	questDB.PlayerDbId = it->second->_questInfo.playerdbid();

	//  DB 요청
	DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_CompleteQuest, player, questDB, itemDBs);
}

void GameRoom::HandleUpdateQuest(shared_ptr<Player> player, PROTOCOL::C_UPDATE_QUEST fromPkt)
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
	player->UpdateQuest(QuestType::QUEST_TYPE_INTERACT, object->_info.typetemplateid(), fromPkt.questinfo().progress());
}



/*-----------------------------------------------------------------------------
	DB
---------------------------------------------------------------------------*/

void GameRoom::DB_Response_UseItem(shared_ptr<Player> player, ItemDB itemDB)
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
		PROTOCOL::S_USE_ITEM toPkt;
		toPkt.mutable_item()->set_slot(itemDB.Slot);
		toPkt.mutable_item()->set_count(itemDB.Count);
		
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}

	// 딜리트 성공
	else if (itemDB.dbState == DB_STATE::DELETE_SUCCESS) {
		// 인벤토리에서 아이템 제거
		player->_inven.Remove(itemDB.Slot);

		// 제거 패킷
		PROTOCOL::S_REMOVE_ITEM removePkt;
		removePkt.add_slots(itemDB.Slot);

		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(removePkt);
		player->_ownerSession->SendPacket(sendBuffer);
	}
	
	// 아이템 사용 효과
	player->UseItem(item->_itemInfo.templateid());

	// 슬롯 락 해제
	player->_inven.SetSlotDBLock(itemDB.Slot, false);
}

void GameRoom::DB_Response_AddItem(shared_ptr<Player> player, ItemDB itemDB)
{
	player->AddItem(itemDB);
}

// DB - 퀘스트
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
	// 성공
	if (questDB.Completed == true) {

		// 퀘스트 업데이트
		player->CompleteQuest(questDB);

		// 아이템 지급
		for (ItemDB& itemDB : itemDBs) 
			player->AddItem(itemDB);

		// Exp 업데이트
		player->AddExp(DataManager::Instance()->_questTable[questDB.TemplateId]->rewardExp);
	}

	// 실패
	else {

	}

	// 슬롯 락 해제
	for (ItemDB& itemdb : itemDBs)
		player->_inven.SetSlotDBLock(itemdb.Slot, false);
}

void GameRoom::Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer)
{
	// 브로드캐스트할 존 추출
	vector<shared_ptr<Zone>> zones;
	GetAdjacentZone(locX, locY, zones);

	// 추출한 존의 모든 플레이어에게 브로드캐스팅
	for (shared_ptr<Zone>& z : zones) 
		for (shared_ptr<Player> p : z->_players) 
			p->_ownerSession->SendPacket(sendBuffer);

}

void GameRoom::BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer)
{
	// 브로드캐스트할 존 추출
	vector<shared_ptr<Zone>> zones;
	GetAdjacentZone(player->_info.pos().location().x(), player->_info.pos().location().y(), zones);

	// 추출한 존에서 본인 제외 모든 플레이어에게 브로드캐스팅
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
	// 쿨타임 등 체크 OK - 일단 패스
	
	// 공격자 위치를 기반으로 공격 패킷 브로드캐스팅
	shared_ptr<GameObject> attacker = ObjectManager::Instance()->Find(fromPkt.attacker());
	if (attacker == nullptr)
		return;

	// 공격 패킷
	PROTOCOL::S_SKILL toPkt;
	toPkt.set_attacker(fromPkt.attacker());
	toPkt.set_skillid(fromPkt.skillid());
	for (int victim : fromPkt.victims())
		toPkt.add_victims(victim);
	
	// 지역 브로드캐스팅
	shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(attacker->_info.pos().location().x(), attacker->_info.pos().location().y(), sendBuffer);


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

				float dx = gameObject->_info.pos().location().x() - victim->_info.pos().location().x();
				float dy = gameObject->_info.pos().location().y() - victim->_info.pos().location().y();

				// TODO : 수정, 사거리 안에 드는지
				float simpleDT = GetDistanceXYSimple(
					victim->_info.pos().location().x() - gameObject->_info.pos().location().x(),
					victim->_info.pos().location().y() - gameObject->_info.pos().location().y());
				if (simpleDT <= pow(180, 2)) {
					
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

void GameRoom::PlayerChat(shared_ptr<Player> player, PROTOCOL::C_CHAT fromPkt)
{
	// 패킷 생성
	PROTOCOL::S_CHAT toPkt;
	toPkt.mutable_object()->set_name(player->_info.name());
	toPkt.set_text(fromPkt.text());

	// 브로드캐스트
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(player->_info.pos().location().x(), player->_info.pos().location().y(), sendBuffer);
}

float GameRoom::GetPlayerAround(shared_ptr<Monster> monster)
{
	// 몬스터의 위치
	const float monsterX = monster->_info.pos().location().x();
	const float monsterY = monster->_info.pos().location().y();

	// 몬스터 탐색 범위 - (간략, 유클리드x,)
	float dtMonsterNoticeSimple = pow(monster->_info.stat().noticedistance(), 2);
	
	// 탐색할 존 = 해당 존과 주변 존들
	vector<shared_ptr<Zone>> zones;
	GetAdjacentZone(monsterX, monsterY, zones);

	// 탐색할 존에서 플레이어 색출
	for (shared_ptr<Zone>& z : zones) {
		//
		for (shared_ptr<Player> p : z->_players) {
			// 몬스터와 플레이어 간 거리가 가장 짧은 녀석으로 선택
			float dtToTargetSimple = GetDistanceXYSimple(
				p->_info.pos().location().x() - monsterX, p->_info.pos().location().y() - monsterY);

			// 몬스터 탐색 범위(간략)과 몬스터플레이어간거리(간략) 비교
			if (dtToTargetSimple <= dtMonsterNoticeSimple) {
				// 몬스터 타겟 설정
				monster->_target = p;

				// 실제값 반환
				return sqrt(dtToTargetSimple);
			}
		}
	}
	
	// 플레이어 없으면 음수값 리턴
	return -1.f;
}

PROTOCOL::PFVector GameRoom::CalculatePathFind(shared_ptr<Monster>& monster, vector<PROTOCOL::PFVector>& paths)
{
	// 이동 가능 거리
	// float deltaTime = (float)(monster->_moveDelta / 1000.f);
	// monster->_moveDelta = 0;
	
	float deltaTime = (float)(monster->_nowUpdateTime - monster->_lastUpdateTime) / 1000.f;
	float dtMovable = deltaTime * monster->_info.stat().speed();

	// 현 위치
	PROTOCOL::PFVector nowPos = monster->_info.pos().location();

	// 이동 확정 지점
	PROTOCOL::PFVector destPos;
	
	// 경로 추적 및 계산
	for (int i = 0; i < paths.size(); i++) {
		// 현 위치에서 다음 위치로의 이동 벡터
		PROTOCOL::PFVector moveV;
		SubV(&moveV, &paths[i], &nowPos);

		// 벡터 크기
		float dt = GetDistance(moveV.x(), moveV.y(), moveV.z());

		// 남은 이동 가능 거리 - 현재 거리
		float perDT = dtMovable - dt;

		// 더 이동 가능
		if (perDT >= 0) {
			// 위치 이동, 이동 가능 거리 업데이트
			nowPos = paths[i];
			destPos = nowPos;
			
			// 남은 이동 거리 업데이트
			dtMovable -= dt;
		}

		// 이동 불가
		else {
			// 방향 * 남은 이동 거리
			Normalize(&moveV);
			moveV.set_x(moveV.x() * dtMovable);
			moveV.set_y(moveV.y() * dtMovable);
			moveV.set_z(moveV.z() * dtMovable);

			// 최종 위치 결정 = 현 위치 + 이동 벡터
			SumV(&nowPos, &nowPos, &moveV);
			destPos = nowPos;

			break;
		}
	}

	return destPos;
}

void GameRoom::ActorMove(shared_ptr<GameObject> gameObject, PROTOCOL::PFVector destPos)
{
	// 현 위치
	float nowX = gameObject->_info.pos().location().x();
	float nowY = gameObject->_info.pos().location().y();
	shared_ptr<Zone>& nowZone = GetZone(nowX, nowY);

	// 이동 위치
	float afterX = destPos.x();
	float afterY = destPos.y();
	shared_ptr<Zone>& afterZone = GetZone(afterX, afterY);

	// 존 번호가 다르면 존 변경
	if (nowZone->_zoneId != afterZone->_zoneId) {
		afterZone->Add(gameObject);
		nowZone->Remove(gameObject);
	}

	// 위치 갱신
	gameObject->_info.mutable_pos()->mutable_location()->CopyFrom(destPos);
	
	// 브로드캐스트
	// 패킷
	PROTOCOL::S_MOVE toPkt;
	toPkt.mutable_object()->CopyFrom(gameObject->_info);

	// 전송
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

