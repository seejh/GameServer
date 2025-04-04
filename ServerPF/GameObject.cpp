#include "pch.h"
#include "GameObject.h"

#include<random>
#include"DataManager.h"
#include"ObjectManager.h"
#include"Item.h"
#include"GameClientPacketHandler.h"
#include"GameRoom.h"
#include"State.h"
#include"GameSession.h"

#include"GameDBManager.h"
#include"DBConnectionPool.h"
#include"GenProcedures.h"
#include"DBManager.h"
#include"QuestManager.h"
#include"Quest.h"

/*------------------------------------------------------------------------------
	GameObject
-------------------------------------------------------------------------------*/

GameObject::GameObject()
{
	_reservedJobs.resize(OBJECT_JOB_SIZE);
}

GameObject::~GameObject()
{
}

void GameObject::OnDamaged(shared_ptr<GameObject> attacker, int damage)
{
	// HP 추출, 대미지 적용
	int hp = _info.stat().hp();
	int realDamage = max(damage - _totalDefence, 0);
	
	hp = max(hp - realDamage, 0);

	_info.mutable_stat()->set_hp(hp);

	// S_ChangeHp 패킷
	PROTOCOL::S_CHANGE_HP toPkt;
	toPkt.set_object(_info.objectid());
	toPkt.set_hp(hp);
	
	// 지역 브로드캐스팅
	shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	if (_ownerRoom != nullptr) {
		_ownerRoom->Broadcast(_info.pos().location().x(), _info.pos().location().y(), sendBuffer);

		// DEAD
		if (hp <= 0)
			OnDead(attacker);
	}
}

void GameObject::OnDead(shared_ptr<GameObject> attacker)
{
	// S_Die 패킷 (공격자, 피해자 설정) 지역 브로드캐스팅
	PROTOCOL::S_DIE toPkt;
	toPkt.set_attacker(attacker->_info.objectid());
	toPkt.set_victim(_info.objectid());
	shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerRoom->Broadcast(_info.pos().location().x(), _info.pos().location().y(), sendBuffer);
	
	// 방 퇴장
	_ownerRoom->LeaveRoom(_info.objectid());
}

void GameObject::OnLeaveGame()
{
	// TODO : 
	// Die는 죽는 모션 재생을 위해 필요한 패킷이며
	// 실제 죽은 객체를 룸에서 제거했다는 것은 LeaveRoom 패킷으로 처리해야한다.

	// 예약 작업 취소 (제일 선행)
	for (weak_ptr<Job>& reservedJob : _reservedJobs) {
		shared_ptr<Job> job = reservedJob.lock();
		if (job)
			job->_cancel = true;
	}
}

void GameObject::OnEnterGame()
{
}

/*------------------------------------------------------------------------------
	플레이어
-------------------------------------------------------------------------------*/

Player::Player()
{
	_questManager = make_shared<QuestManager>();
}

Player::~Player()
{
	cout << "~Player()" << endl;
}

void Player::OnDamaged(shared_ptr<GameObject> attacker, int damage)
{
	GameObject::OnDamaged(attacker, damage);
}

void Player::OnDead(shared_ptr<GameObject> attacker)
{
	GameObject::OnDead(attacker);

	// 공격자가 몬스터면 타겟 해제
	if (PROTOCOL::GameObjectType::MONSTER == ObjectManager::Instance()->GetObjectTypeById(attacker->_info.objectid())) {
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(attacker);
		monster->_target.reset();
	}
}

void Player::OnLeaveGame()
{
	// 예약 작업(Job) 취소, 플레이어 정보 DB 저장
	GameObject::OnLeaveGame();

	// 봇이 아니라 실제 플레이어면 DB작업
	if (_isBot == false) {
		DBConnection* dbConn = DBManager::Instance()->_gameDbManager->Pop();

		// 스텟
		SP::UpdatePlayer updatePlayer(*dbConn);
		updatePlayer.In_PlayerDbId(_playerDbId);
		updatePlayer.In_Level(_info.stat().level());
		updatePlayer.In_TotalExp(_info.stat().exp());
		updatePlayer.In_MaxHp(_info.stat().maxhp());
		updatePlayer.In_Hp(_info.stat().hp());
		updatePlayer.In_Damage(_info.stat().damage());
		updatePlayer.In_LocationX(_info.pos().location().x());
		updatePlayer.In_LocationY(_info.pos().location().y());
		updatePlayer.In_LocationZ(_info.pos().location().z());
		if (updatePlayer.Execute() == false)
			cout << "[PLAYER-" << _info.objectid() << "] OnLeaveRoom Error - UpdatePlayer Execute Failed" << endl;

		// 아이템 : 아이템은 일단 실시간 반영되고 있어서 패스

		// 퀘스트
		for (auto p : _questManager->_quests) {
			if (p.second->_questInfo.completed())
				continue;

			SP::UpdateQuest updateQuest(*dbConn);
			updateQuest.In_QuestDbId(p.second->_questInfo.questdbid());
			updateQuest.In_TemplateId(p.second->_questInfo.templateid());
			updateQuest.In_PlayerDbId(p.second->_questInfo.playerdbid());
			updateQuest.In_Completed(p.second->_questInfo.completed());
			updateQuest.In_Progress(p.second->_questInfo.progress());
			if (updateQuest.Execute() == false) 
				cout << "[PLAYER-" << _info.objectid() << "] OnLeaveRoom Error - UpdateQuest Execute Failed" << endl;
		}

		DBManager::Instance()->_gameDbManager->Push(dbConn);
	}

	// 패킷
	PROTOCOL::S_LEAVE_ROOM toPkt;
	toPkt.mutable_object()->set_objectid(_info.objectid());
	toPkt.mutable_object()->set_objecttype(_info.objecttype());
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);

	// 플레이어 비전 비활성화
	_vision._prevObjects.clear();
	_vision._ownerPlayer.reset();
	
	// 룸 비움
	_ownerRoom.reset();

	// 세션에서 플레이어 참조 제거
	// _ownerSession->_player.reset();
}

void Player::OnEnterGame()
{
	GameObject::OnEnterGame();

	// 아이템 기반으로 스텟 계산
	CalculateAddStat();

	// 당사자에게만 S_EnterRoom
	PROTOCOL::S_ENTER_ROOM toPkt;
	toPkt.set_success(true);
	toPkt.mutable_object()->CopyFrom(_info);
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);

	// 비전 플레이어 활성화
	_vision._ownerPlayer = static_pointer_cast<Player>(shared_from_this());
	_vision.Update();
}

void Player::UseItem(int itemId)
{
	// 아이템 정보를 조회해서 해당 아이템의 효과 적용

	// 아이템 정보 조회
	ItemData* itemData = DataManager::Instance()->_itemTable[itemId];
	if (itemData == nullptr) {
		cout << "Player::UseItem() Error - Can't Find ItemId:" << itemId << "'s itemData" << endl;
		return;
	}
		
	// 소비류가 아니면 패스
	if (itemData->_itemType != PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
		cout << "Player::UseItem() Error - ItemType No Consumable" << endl;
		return;
	}
		
	switch (reinterpret_cast<ConsumableData*>(itemData)->_consumableType) {
	case PROTOCOL::ConsumableType::CONSUMABLE_TYPE_HP_POTION: 
	{
		// HP 포션류
		// 효과 적용 - 회복된 체력 = 현체력 + 아이템 회복력
		int32 afterHP = _info.stat().hp() + reinterpret_cast<ConsumableData*>(itemData)->_recovery;
		_info.mutable_stat()->set_hp(afterHP);

		// 패킷
		PROTOCOL::S_CHANGE_HP toPkt;
		toPkt.set_object(_info.objectid());
		toPkt.set_hp(afterHP);

		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerSession->SendPacket(sendBuffer);
	} 
	break;

	case PROTOCOL::ConsumableType::CONSUMABLE_TYPE_MP_POTION:
		// MP 포션류
		cout << "Player::UseItem(int) Error - MP POTION" << endl;
		break;

	default:
		// 그 외
		cout << "Player::UseItem(int) Error - Not HP, MP POTION" << endl;
		break;
	}
}

void Player::CalculateAddStat()
{
	// 장비와 스텟 변경으로 인한 전체 공, 방 업데이트
	
	//
	_totalDamage = _info.stat().damage();
	_totalDefence = 0;

	_weaponDamage = 0;
	_armorDefence = 0;
	
	// 아이템 목록에서
	for (auto p : _inven._items) { // 새로 아이템 얻은 후로 장착 요청햇는데 p자체가 nullptr -> 어떻게 없을 수가 있냐
		// 아이템
		shared_ptr<Item> item = p.second;
		if (item == nullptr) {
			cout << "Player::CalculateAddStat() Error - Item Nullptr Slot : " << p.first << endl;
			continue;
		}

		// 미착용 패스
		if (item->_itemInfo.equipped() == false) 
			continue;
			
		// 착용된 장비들 - 공,방 합산
		if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_WEAPON)
			_weaponDamage += static_pointer_cast<Weapon>(item)->_damage;
		else if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR)
			_armorDefence += static_pointer_cast<Armor>(item)->_defence;
	}

	// 전체 공,방 스텟에 장비스텟 합산
	_totalDamage += _weaponDamage;
	_totalDefence += _armorDefence;
}

void Player::AddExp(int exp)
{
	// 해당 레벨의 레벨업 경험치(Max)
	int32 maxExp = DataManager::Instance()->_statTable[_info.stat().level()].totalExp;

	// 변경된 경험치 = 현재 exp + 추가 exp
	_info.mutable_stat()->set_exp(_info.stat().exp() + exp);
	
	// Exp 패킷
	PROTOCOL::S_ADD_EXP toPkt;
	toPkt.set_exp(_info.stat().exp());

	// Exp 전송
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);

	// 레벨업
	if (_info.stat().exp() >= maxExp) {
		
		// 다음 레벨 스텟
		StatData nextStat = DataManager::Instance()->_statTable[_info.stat().level() + 1];
		_info.mutable_stat()->set_level(nextStat.level);
		_info.mutable_stat()->set_maxhp(nextStat.maxhp);
		_info.mutable_stat()->set_hp(nextStat.maxhp);
		_info.mutable_stat()->set_damage(nextStat.damage);
		//_info.mutable_stat()->set_exp();
		_info.mutable_stat()->set_totalexp(nextStat.totalExp);

		// 레벨업 패킷
		PROTOCOL::S_LEVEL_UP toPkt2;
		toPkt2.mutable_info()->mutable_stat()->CopyFrom(_info.stat());

		// 레벨업 패킷 전송
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt2);
		_ownerSession->SendPacket(sendBuffer);
	}
}

ItemDB Player::CanAddItemDB(int itemId, int count)
{
	// 업데이트, 인서트 / 슬롯
	ItemDB itemDB;
	
	// 소비류
	if (DataManager::Instance()->_itemTable[itemId]->_itemType == PROTOCOL::ITEM_TYPE_CONSUMABLE) {
		itemDB.Slot = _inven.GetStackPos(itemId);
		// 스택 불가능
		if (itemDB.Slot == -1) {
			// 새 슬롯 확인
			itemDB.Slot = _inven.GetEmptySlot();
			if (itemDB.Slot != -1) {
				itemDB.TemplateId = itemId;
				itemDB.Count = count;
				// itemDB.Slot = ;
				itemDB.Equipped = false;
				itemDB.PlayerDbId = _info.playerdbid();
				itemDB.dbState = DB_STATE::INSERT_NEED;

				if (_inven.SetSlotDBLock(itemDB.Slot, true) == false) {
					// 현재 슬롯 락, 작업 취소
					itemDB.Slot = -1;
				}
			}
		}
		// 스택 가능
		else {
			shared_ptr<Item>& item = _inven._items[itemDB.Slot];

			itemDB.ItemDbId = item->_itemInfo.itemdbid();
			itemDB.TemplateId = item->_itemInfo.templateid();
			itemDB.Count = item->_itemInfo.count() + count;
			itemDB.Slot = item->_itemInfo.slot();
			itemDB.Equipped = item->_itemInfo.equipped();
			itemDB.PlayerDbId = item->_itemInfo.playerdbid();
			itemDB.dbState = DB_STATE::UPDATE_NEED;

			if (_inven.SetSlotDBLock(itemDB.Slot, true) == false) {
				// 현재 슬롯 락, 작업 취소
				itemDB.Slot = -1;
			}
		}
	}

	// 장비류
	else {
		itemDB.Slot = _inven.GetEmptySlot();
		if (itemDB.Slot != -1) {
			itemDB.TemplateId = itemId;
			itemDB.Count = count;
			// itemDB.Slot = ;
			itemDB.Equipped = false;
			itemDB.PlayerDbId = _info.playerdbid();
			itemDB.dbState = DB_STATE::INSERT_NEED;

			if (_inven.SetSlotDBLock(itemDB.Slot, true) == false) {
				// 현재 슬롯 락, 작업 취소
				itemDB.Slot = -1;
			}
		}
	}

	return itemDB;
}

void Player::AddItem(ItemDB itemDB)
{
	// 생성
	if (itemDB.dbState == DB_STATE::INSERT_SUCCESS) {
		// 아이템 생성
		shared_ptr<Item> item = Item::MakeItem(itemDB);
		if (item != nullptr) {
			_inven.Add(item);

			// 생성 패킷
			PROTOCOL::S_ADD_ITEM toPkt;
			toPkt.add_items()->CopyFrom(item->_itemInfo);
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
			_ownerSession->SendPacket(sendBuffer);
		}
	}
	// 업데이트
	else if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		// 아이템 업데이트
		shared_ptr<Item>& item = _inven._items[itemDB.Slot];
		if (item != nullptr) {
			item->_itemInfo.set_itemdbid(itemDB.ItemDbId);
			item->_itemInfo.set_playerdbid(itemDB.PlayerDbId);
			item->_itemInfo.set_templateid(itemDB.TemplateId);
			item->_itemInfo.set_count(itemDB.Count);
			item->_itemInfo.set_slot(itemDB.Slot);
			item->_itemInfo.set_equipped(itemDB.Equipped);

			// 업데이트 패킷
			PROTOCOL::S_UPDATE_ITEM toPkt;
			toPkt.add_items()->CopyFrom(item->_itemInfo);
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
			_ownerSession->SendPacket(sendBuffer);
		}
	}
	// 
	else {

	}

	// 슬롯 락 해제
	_inven.SetSlotDBLock(itemDB.Slot, false);
}

void Player::CompleteQuest(QuestDB questDB)
{
	if (questDB.dbState != DB_STATE::UPDATE_SUCCESS)
		return;

	auto it = _questManager->_quests.find(questDB.TemplateId);
	if (it != _questManager->_quests.end()) {
		// 업데이트
		it->second->_questInfo.set_questdbid(questDB.QuestDbId);
		it->second->_questInfo.set_playerdbid(questDB.PlayerDbId);
		it->second->_questInfo.set_templateid(questDB.TemplateId);
		it->second->_questInfo.set_progress(questDB.Progress);
		it->second->_questInfo.set_completed(questDB.Completed);

		// 패킷
		PROTOCOL::S_COMPLETE_QUEST toPkt;
		toPkt.set_questid(questDB.TemplateId);
		toPkt.set_result(true);
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerSession->SendPacket(sendBuffer);
	}
}

void Player::RemoveQuest(QuestDB questDB)
{
	if (questDB.dbState != DB_STATE::DELETE_SUCCESS)
		return;

	auto it = _questManager->_quests.find(questDB.TemplateId);
	if (it != _questManager->_quests.end()) {
		// 제거
		_questManager->Remove(questDB.TemplateId);

		// 패킷
		PROTOCOL::S_REMOVE_QUEST toPkt;
		toPkt.set_questid(questDB.TemplateId);
		toPkt.set_result(true);
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerSession->SendPacket(sendBuffer);
	}
}

void Player::AddQuest(QuestDB questDB)
{
	if (questDB.dbState != DB_STATE::INSERT_SUCCESS)
		return;

	// 생성
	shared_ptr<Quest> quest = Quest::MakeQuest(questDB);
	_questManager->Add(quest);

	// 패킷
	PROTOCOL::S_ADD_QUEST toPkt;
	toPkt.mutable_quest()->set_questdbid(questDB.QuestDbId);
	toPkt.mutable_quest()->set_playerdbid(questDB.PlayerDbId);
	toPkt.mutable_quest()->set_templateid(questDB.TemplateId);
	toPkt.mutable_quest()->set_progress(questDB.Progress);
	toPkt.mutable_quest()->set_completed(questDB.Completed);
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);
}

void Player::UpdateQuest(QuestType questType, int objectiveId, int quantity)
{
	for (auto p : _questManager->_quests) {
		if (p.second->_questInfo.completed() == true)
			continue;

		if (p.second->_questData->type != questType)
			continue;

		if (p.second->_questData->objectiveId != objectiveId)
			continue;

		if (p.second->_questInfo.progress() >= p.second->_questData->quantity)
			continue;

		// (서버 메모리) 업데이트
		p.second->_questInfo.set_progress(p.second->_questInfo.progress() + quantity);

		//
		QuestDB questDB;
		questDB.QuestDbId = p.second->_questInfo.questdbid();
		questDB.PlayerDbId = p.second->_questInfo.playerdbid();
		questDB.TemplateId = p.second->_questInfo.templateid();
		questDB.Progress = p.second->_questInfo.progress();
		questDB.Completed = p.second->_questInfo.completed();

		// DB 
		DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::TransactNoti_UpdateQuest, static_pointer_cast<Player>(shared_from_this()), questDB);

		// 패킷
		PROTOCOL::S_UPDATE_QUEST toPkt;
		toPkt.mutable_questinfo()->CopyFrom(p.second->_questInfo);
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerSession->SendPacket(sendBuffer);
	}
}

void Player::SaveToDB(bool init)
{
	// 테스트 : 일단 현재 30초 단위
	uint64 saveTick = 3 * 10'000;
	
	// SAVE
	if (init == false) {
		// 플레이어 정보(스텟, 위치)
		
		
		// 인벤토리

	}

	// 재등록
	_reservedJobs[PLAYERJOBS::PLAYER_JOB_SAVE] = _ownerRoom->DoTimer(saveTick, [this]() { SaveToDB(false); });
}

/*--------------------------------------------------------------------
	몬스터
-------------------------------------------------------------------*/

Monster::Monster() : _currentState(StateIdle::Instance())
{
}

void Monster::Update()
{
	// 델타
	/*uint64 nowTime = GetTickCount64();
	_deltaTime = (float)(nowTime - _lastUpdateTime) / 1000;
	_lastUpdateTime = nowTime;*/

	// 
	_lastUpdateTime = _nowUpdateTime;
	_nowUpdateTime = GetTickCount64();

	// 상태에 따른 Update
	if (_currentState)
		_currentState->Execute(static_pointer_cast<Monster>(shared_from_this()));
	
	// 몬스터 Update 재등록, 0.2초 주기
	_reservedJobs[MONSTERJOBS::MONSTER_JOB_UPDATE] = _ownerRoom->DoTimer(200, [this]() { Update(); });
}

void Monster::OnDamaged(shared_ptr<GameObject> attacker, int damage)
{
	GameObject::OnDamaged(attacker, damage);
}

void Monster::OnDead(shared_ptr<GameObject> attacker)
{
	// 예약 작업 취소, 플레이어 보상, 재입장 예약
	GameObject::OnLeaveGame();

	shared_ptr<GameRoom> room = _ownerRoom;
	
	GameObject::OnDead(attacker);

	// 공격자 == 플레이어
	if (attacker->_info.objecttype() == PROTOCOL::GameObjectType::PLAYER) {

		// 봇일시 패스 - TEST
		if (static_pointer_cast<Player>(attacker)->_isBot == false) {

			// 몬스터 데이터 조회
			MonsterData* monsterData = DataManager::Instance()->_monsterTable[_info.typetemplateid()];
			if (monsterData == nullptr)
				return;

			// 경험치 보상
			static_pointer_cast<Player>(attacker)->AddExp(monsterData->stat.totalexp());

			// 아이템 보상
			RewardData reward = GetRandomReward();
			if (reward.itemId != 0) {
				// static_pointer_cast<Player>(attacker)->TakeReward(reward.itemId, reward.count);

				ItemDB itemDB = static_pointer_cast<Player>(attacker)->CanAddItemDB(reward.itemId, reward.count);
				if (itemDB.Slot != -1) 
					if (static_pointer_cast<Player>(attacker)->_inven.SetSlotDBLock(itemDB.Slot, true) == true)
						DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_AddItem, static_pointer_cast<Player>(attacker), itemDB);
			}
			
			// 퀘스트 업데이트
			static_pointer_cast<Player>(attacker)->UpdateQuest(QuestType::QUEST_TYPE_KILL, _info.typetemplateid(), 1);
		}
	}

	// 재입장 예약
	_info.mutable_pos()->mutable_location()->CopyFrom(_basePos);

	_info.mutable_stat()->set_hp(_info.stat().maxhp());
	_currentState = StateIdle::Instance();

	room->DoTimer(10000, &GameRoom::EnterRoom, shared_from_this());
}

void Monster::OnLeaveGame()
{
	//
	GameObject::OnLeaveGame();

	// 몬스터의 방 참조 제거
	_ownerRoom.reset();
}

void Monster::OnEnterGame()
{
	GameObject::OnEnterGame();

	Update();
}

void Monster::Init(int templateId)
{
	// 몬스터 데이터 조회
	MonsterData* data = DataManager::Instance()->_monsterTable[templateId];
	if (data == nullptr) {
		cout << "Monster-" << _info.objectid() << " Init() Error : No MonsterTable" << endl;
		return;
	}

	// 몬스터 정보 세팅
	_info.set_typetemplateid(data->id);
	_info.set_objecttype(PROTOCOL::GameObjectType::MONSTER);
	_info.set_name(data->name);
	_info.mutable_stat()->CopyFrom(data->stat);
	_totalDamage = _info.stat().damage();
	
	// 몬스터 상태 세팅
	_currentState = StateIdle::Instance();

	// 
	_lastUpdateTime = GetTickCount64();
}

void Monster::ChangeState(State* state)
{
	// 현상태 EXIT
	_currentState->Exit(static_pointer_cast<Monster>(shared_from_this()));

	// 상태 변경
	_currentState = state;

	// 변경된 상태 Enter
	_currentState->Enter(static_pointer_cast<Monster>(shared_from_this()));
}

void Monster::Skill()
{
	// 타겟 추출
	shared_ptr<Player> target = _target.lock();
	if (target != nullptr && _ownerRoom != nullptr) {
		// S_Skill 패킷
		PROTOCOL::S_SKILL toPkt;

		// 피해자 설정
		toPkt.add_victims(target->_info.objectid());
		
		// 공격자 설정
		toPkt.set_attacker(_info.objectid());
		
		// 스킬 설정
		toPkt.set_skillid(PROTOCOL::SkillType::SKILL_AUTO);

		// 스킬 쿨 계산
		_nextAttackTime = _nowUpdateTime + _info.stat().attackcooltime();

		// S_Skill 패킷 지역 브로드캐스팅
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerRoom->Broadcast(_info.pos().location().x(), _info.pos().location().y(), sendBuffer);

		// 피격 업데이트
		// 스킬 조회
		auto p = DataManager::Instance()->_skillTable.find(PROTOCOL::SkillType::SKILL_AUTO);
		if (p == DataManager::Instance()->_skillTable.end())
			return;

		// 대미지 계산 (스킬 댐 + 스텟) 
		int damage = p->second.damage + _totalDamage;
		
		// 피격 적용
		target->OnDamaged(static_pointer_cast<Monster>(shared_from_this()), damage);
	}
}

RewardData Monster::GetRandomReward()
{
	// 확률
	int ran = rand() % 100;

	// 몬스터 보상 목록
	MonsterData* monsterData = DataManager::Instance()->_monsterTable[_info.typetemplateid()];
	if (monsterData == nullptr) {
		cout << "Monster::GetRandomReward() Error : No MonsterData" << endl;
		return RewardData();
	}

	// 보상목록에서 드랍 확인
	for (RewardData reward : monsterData->rewardDatas) {
		if (ran <= reward.itemDropRate) 
			return reward;
	}

	// 드랍X
	return RewardData();
}

PROTOCOL::PFVector Monster::GetRandomPatrolPos()
{
	// basePos 기준으로 일정 반경 랜덤위치
	
	// FVector patrolPos;
	PROTOCOL::PFVector patrolPos;

	// 반경
	int radius = 450;
	
	// 
	int symbol;
	int pos;

	// X
	symbol = rand() % 2;
	pos = rand() % radius;
	patrolPos.set_x(_basePos.x() + ((symbol == 0) ? pos * -1 : pos));
	// patrolPos._x = _basePos._x + ((symbol == 0) ? pos * -1 : pos);
	
	// Y
	symbol = rand() % 2;
	pos = rand() % radius;
	patrolPos.set_y(_basePos.y() + ((symbol == 0) ? pos * -1 : pos));
	// patrolPos._y = _basePos._y + ((symbol == 0) ? pos * -1 : pos);
	
	// Z
	// patrolPos._z = _info.pos().locationz();
	patrolPos.set_z(_basePos.z());

	return patrolPos;
}

//bool Monster::GetNextPos(OUT PROTOCOL::ObjectInfo& nextPos)
//{
//	//
//	int test = 0;
//	if (_ownerRoom == nullptr) {
//		cout << "Monster::GetNextPos() _ownerRoom == nullptr, set now pos" << endl;
//		
//		nextPos.mutable_pos()->set_locationx(_info.pos().locationx());
//		nextPos.mutable_pos()->set_locationy(_info.pos().locationy());
//
//		return false;
//	}
//
//	// 타겟할 위치
//	float targetX;
//	float targetY;
//
//	// 리턴, 추적, 순찰
//	// 리턴일 때
//	if (_currentState->GetType() == MONSTER_STATE::RETURN) {
//		targetX = _basePos.x();
//		targetY = _basePos.y();
//
//		test = 1;
//	}
//
//	// 추적일 때
//	else if (_currentState->GetType() == MONSTER_STATE::CHASE) {
//		shared_ptr<Player> target = _target.lock();
//		if (target == nullptr) {
//			cout << "Monster::GetNextPos() Error - StateChase Invalid Target" << endl;
//			return false;
//		}
//
//		targetX = target->_info.pos().locationx();
//		targetY = target->_info.pos().locationy();
//
//		test = 2;
//	}
//
//	// 순찰
//	else {
//		targetX = _patrolPos.x();
//		targetY = _patrolPos.y();
//
//		test = 3;
//	}
//
//	///////////////////////////////////////////////// 계산
//	
//	// 이동 벡터 (각 성분)
//	float moveVX = targetX - _info.pos().locationx();
//	float moveVY = targetY - _info.pos().locationy();
//	if (moveVX == 0 && moveVY == 0) {
//		// 타겟 지점과 같음, 이동 필요 X
//		nextPos.mutable_pos()->set_locationx(_info.pos().locationx());
//		nextPos.mutable_pos()->set_locationy(_info.pos().locationy());
//
//		return true;
//	}
//
//	// 이동 벡터 크기
//	float dt = _ownerRoom->DistanceToTarget(moveVX, moveVY);
//
//	// 방향 벡터 (각 성분)
//	float dirX = moveVX / dt;
//	float dirY = moveVY / dt;
//
//	// 최대 이동 가능 위치
//	float x = dirX * _info.stat().speed() * _deltaTime;
//	float y = dirY * _info.stat().speed() * _deltaTime;
//
//	// 목적지 < 최대 이동 가능 거리 -> 목적지를 좌표로 설정
//	if (abs(targetX) < abs(x)) x = targetX;
//	if (abs(targetY) < abs(y)) y = targetY; // 주목할 필요
//
//	// 이동 처리
//	nextPos.mutable_pos()->set_locationx(_info.pos().locationx() + x);
//	nextPos.mutable_pos()->set_locationy(_info.pos().locationy() + y);
//	nextPos.mutable_pos()->set_locationz(_info.pos().locationz());
//
//	return true;
//
//}


/*
	
*/

void Npc::OnDamaged(shared_ptr<GameObject> attacker, int damage)
{
}

void Npc::OnDead(shared_ptr<GameObject> attacker)
{
}

void Npc::OnLeaveGame()
{
	// 일단 기능없음
	GameObject::OnLeaveGame();
}

void Npc::OnEnterGame()
{
	GameObject::OnEnterGame();
}

void Npc::Init(int templateId)
{
	auto it = DataManager::Instance()->_npcTable.find(templateId);
	if (it != DataManager::Instance()->_npcTable.end()) {
		NpcData* data = it->second;
		
		_info.set_typetemplateid(templateId);
		_info.set_objecttype(PROTOCOL::GameObjectType::NPC);
		_info.set_name(data->name);
	}
}

