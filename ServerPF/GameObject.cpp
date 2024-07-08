#include "pch.h"
#include "GameObject.h"

#include<random>
#include"DataManager.h"
#include"ObjectManager.h"
#include"Item.h"
#include"ClientPacketHandler.h"
#include"Room.h"
#include"State.h"

#include"DBTransaction.h"
#include"DBConnectionPool.h"
#include"GenProcedures.h"


/*------------------------------------------------------------------------------
	GameObject
-------------------------------------------------------------------------------*/

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
	PROTOCOL::S_ChangeHp toPkt;
	toPkt.set_object(_info.objectid());
	toPkt.set_hp(hp);
	
	// 브로드캐스팅
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerRoom->Broadcast(_info.pos().locationx(), _info.pos().locationy(), sendBuffer);

	// DEAD
	if (hp <= 0) 
		OnDead(attacker);
}

void GameObject::OnDead(shared_ptr<GameObject> attacker)
{
	// S_Die 패킷
	PROTOCOL::S_Die toPkt;
	
	// 공격자 설정
	toPkt.set_attacker(attacker->_info.objectid());
	
	// 피해자 설정
	toPkt.set_victim(_info.objectid());
	
	// S_Die 패킷 브로드캐스팅
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerRoom->Broadcast(_info.pos().locationx(), _info.pos().locationy(), sendBuffer);
	
	// 방 퇴장
	_ownerRoom->LeaveRoom(_info.objectid());
}

/*------------------------------------------------------------------------------
	플레이어
-------------------------------------------------------------------------------*/

Player::Player()
{}

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

void Player::UseItem(PROTOCOL::C_UseItem fromPkt)
{
	// 아이템 추출
	shared_ptr<Item> item = _inven.Get(fromPkt.itemdbid());
	if (item == nullptr)
		return;
	
	// 소모품
	if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
		//
		DBConnection* dbConn = GDBConnectionPool->Pop();

		//
		shared_ptr<Consumable> consumable = static_pointer_cast<Consumable>(item);
		if (consumable == nullptr)
			return;

		// 스펙조회
		ConsumableData* consumableData = reinterpret_cast<ConsumableData*>(DataManager::Instance()->_itemTable[item->_itemInfo.templateid()]);
		if (consumableData != nullptr) {

			// 플레이어 체력 변경
			int32 maxhp = _info.stat().maxhp();
			int32 recoveredHp = _info.stat().hp() + consumableData->_recovery;

			_info.mutable_stat()->set_hp(((maxhp < recoveredHp) ? maxhp : recoveredHp));

			// 아이템 카운트 - 1
			consumable->_itemInfo.set_count(consumable->_itemInfo.count() - 1);

			// 소모품 모두 사용 -> 삭제
			if (consumable->_itemInfo.count() <= 0) {
				// DB - 제거
				SP::DeleteItem deleteItem(*dbConn);
				deleteItem.In_ItemDbId(consumable->_itemInfo.itemdbid());

				deleteItem.Execute();

				// 서버 메모리 제거
				_inven.Remove(consumable->_itemInfo.itemdbid());
			}

			// 소모품 여유분 남아 있음 -> 업데이트
			else {
				// DB - 변경
				SP::UpdateItem updateItem(*dbConn);
				updateItem.In_ItemDbId(consumable->_itemInfo.itemdbid());
				updateItem.In_TemplateId(consumable->_itemInfo.templateid());
				updateItem.In_Count(consumable->_itemInfo.count());
				updateItem.In_Slot(consumable->_itemInfo.slot());
				updateItem.In_Equipped(consumable->_itemInfo.equipped());
				updateItem.In_PlayerDbId(_info.playerdbid());
	
				updateItem.Execute();
			}

			//
			GDBConnectionPool->Push(dbConn);

			// 아이템사용 패킷
			{
				PROTOCOL::S_UseItem useItemPkt;
				useItemPkt.set_itemdbid(fromPkt.itemdbid());
				useItemPkt.set_use(true);
				
				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(useItemPkt);
				_ownerSession->SendPacket(sendBuffer);
			}

			// HP변경 패킷 - 오브젝트넘버, 체력
			{
				PROTOCOL::S_ChangeHp changeHpPkt;
				changeHpPkt.set_object(_info.objectid());
				changeHpPkt.set_hp(_info.stat().hp());
				
				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(changeHpPkt);
				_ownerSession->SendPacket(sendBuffer);
			}
			
			return;
		}
	}

	// 장비류
	else {
		//
		DBConnection* dbConn = GDBConnectionPool->Pop();

		// 장착 요청인데 해당 부위에 이미 장비가 있으면 장착 해제 처리
		if (fromPkt.use() == true) {
			// 
			shared_ptr<Item> unequipItem;

			// 장착하려는 아이템(item)과 동일 포지션에 장착되어 있는 아이템이 있는가
			unequipItem = _inven.FindEquipped(item);
			if (unequipItem != nullptr) {
				
				// 서버 메모리
				unequipItem->_itemInfo.set_equipped(false);

				// DB 
				SP::UpdateItem updateItem(*dbConn);
				updateItem.In_ItemDbId(unequipItem->_itemInfo.itemdbid());
				updateItem.In_TemplateId(unequipItem->_itemInfo.templateid());
				updateItem.In_Count(unequipItem->_itemInfo.count());
				updateItem.In_Slot(unequipItem->_itemInfo.slot());
				updateItem.In_Equipped(unequipItem->_itemInfo.equipped());
				updateItem.In_PlayerDbId(_info.playerdbid());

				updateItem.Execute();
				
				// 클라측 (전송)
				PROTOCOL::S_UseItem toPktUnUse;
				toPktUnUse.set_itemdbid(unequipItem->_itemInfo.itemdbid());
				toPktUnUse.set_use(false);

				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPktUnUse);
				_ownerSession->SendPacket(sendBuffer);
			}
		}

		// 요청한 장비 착용 or 해제 처리
		// 서버 메모리
		item->_itemInfo.set_equipped(fromPkt.use());

		// DB
		SP::UpdateItem updateItem(*dbConn);
		updateItem.In_ItemDbId(item->_itemInfo.itemdbid());
		updateItem.In_TemplateId(item->_itemInfo.templateid());
		updateItem.In_Count(item->_itemInfo.count());
		updateItem.In_Slot(item->_itemInfo.slot());
		updateItem.In_Equipped(item->_itemInfo.equipped());
		updateItem.In_PlayerDbId(_info.playerdbid());

		updateItem.Execute();

		//
		GDBConnectionPool->Push(dbConn);

		// 클라측 (전송)
		PROTOCOL::S_UseItem toPkt;
		toPkt.set_itemdbid(fromPkt.itemdbid());
		toPkt.set_use(fromPkt.use());
		
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerSession->SendPacket(sendBuffer);
	}

	CalculateAddStat();
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
	for (auto p : _inven._items) {
		// 아이템
		shared_ptr<Item> item = p.second;

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

void Player::OnLeaveGame()
{
	// TODO : 플레이어 정보 DB에 저장

	// DB 커넥션
	DBConnection* dbConn = GDBConnectionPool->Pop();

	// 스텟
	SP::UpdatePlayer updatePlayer(*dbConn);
	updatePlayer.In_PlayerDbId(_playerDbId);
	updatePlayer.In_Level(_info.stat().level());
	updatePlayer.In_TotalExp(_info.stat().exp());
	updatePlayer.In_MaxHp(_info.stat().maxhp());
	updatePlayer.In_Hp(_info.stat().hp());
	updatePlayer.In_Damage(_info.stat().damage());
	updatePlayer.In_LocationX(_info.pos().locationx());
	updatePlayer.In_LocationY(_info.pos().locationy());
	updatePlayer.In_LocationZ(_info.pos().locationz());
	updatePlayer.Execute();

	// 아이템 : 아이템은 일단 실시간 반영되고 있어서 패스


	// DB 커넥션 반납
	GDBConnectionPool->Push(dbConn);
}

void Player::AddExp(int exp)
{
	// 해당 레벨의 레벨업 경험치(Max)
	int32 maxExp = DataManager::Instance()->_statTable[_info.stat().level()].totalExp;

	// 변경된 경험치 = 현재 exp + 추가 exp
	_info.mutable_stat()->set_exp(_info.stat().exp() + exp);
	
	// Exp 패킷
	PROTOCOL::S_AddExp toPkt;
	toPkt.set_exp(_info.stat().exp());

	// Exp 전송
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
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
		PROTOCOL::S_LevelUp toPkt2;
		toPkt2.mutable_info()->mutable_stat()->CopyFrom(_info.stat());

		// 레벨업 패킷 전송
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt2);
		_ownerSession->SendPacket(sendBuffer);
	}
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
	uint64 nowTime = GetTickCount64();
	_deltaTime = (float)(nowTime - _lastUpdateTime) / 1000;
	_lastUpdateTime = nowTime;

	// 상태에 따른 Update
	if (_currentState)
		_currentState->Execute(static_pointer_cast<Monster>(shared_from_this()));
	
	// 몬스터 Update 재등록, 0.2초 주기
	_reservedJob = _ownerRoom->DoTimer(200, [this]() { Update(); });
}

void Monster::OnDamaged(shared_ptr<GameObject> attacker, int damage)
{
	GameObject::OnDamaged(attacker, damage);
}

void Monster::OnDead(shared_ptr<GameObject> attacker)
{
	shared_ptr<Room> room = _ownerRoom;

	GameObject::OnDead(attacker);

	// 공격자 == 플레이어
	if (attacker->_info.objecttype() == PROTOCOL::GameObjectType::PLAYER) {
		// 경험치 보상
		int exp = DataManager::Instance()->_monsterTable[_templateId].stat.totalexp();
		static_pointer_cast<Player>(attacker)->AddExp(exp);

		// 아이템 보상
		RewardData reward = GetRandomReward();
		if (reward.itemId != 0) 
			DBTransaction::Instance()->DoAsync(&DBTransaction::RewardPlayer, static_pointer_cast<Player>(attacker), reward, attacker->_ownerRoom);
	}

	// 재입장 예약
	_info.mutable_pos()->set_locationx(_basePos._x);
	_info.mutable_pos()->set_locationy(_basePos._y);
	_info.mutable_pos()->set_locationz(_basePos._z);

	_info.mutable_stat()->set_hp(_info.stat().maxhp());
	_currentState = StateIdle::Instance();

	room->DoTimer(10000, &Room::EnterRoom, shared_from_this());
}

void Monster::Init(int templateId)
{
	// 몬스터 정보 조회
	if (DataManager::Instance()->_monsterTable.count(templateId) == 0) {
		cout << "Monster-" << _info.objectid() << ", Init Error : No MonsterTable" << endl;
		return;
	}

	MonsterData data = DataManager::Instance()->_monsterTable[templateId];
	
	// 몬스터 정보 세팅
	_templateId = data.id;
	_info.set_objecttype(PROTOCOL::GameObjectType::MONSTER);
	_info.set_name(data.name);
	_info.mutable_stat()->CopyFrom(data.stat);
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

void Monster::Skill(uint64 nowTime)
{
	// 타겟 추출
	shared_ptr<Player> target = _target.lock();
	if (target != nullptr && _ownerRoom != nullptr) {
		// S_Skill 패킷
		PROTOCOL::S_Skill toPkt;

		// 피해자 설정
		toPkt.add_victims(target->_info.objectid());
		
		// 공격자 설정
		toPkt.set_attacker(_info.objectid());
		
		// 스킬 설정
		toPkt.set_skillid(PROTOCOL::SkillType::SKILL_AUTO);

		// 스킬 쿨 계산
		_nextAttackTime = nowTime + _info.stat().attackcooltime();

		// S_Skill 패킷 브로드캐스팅
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerRoom->Broadcast(_info.pos().locationx(), _info.pos().locationy(), sendBuffer);

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
	MonsterData monsterData = DataManager::Instance()->_monsterTable[_templateId];
	for (RewardData reward : monsterData.rewardDatas) {
		
		// 드랍
		if (ran <= reward.itemDropRate) 
			return reward;
	}

	// 드랍X
	return RewardData();
}

FVector Monster::GetRandomPatrolPos()
{
	// 베이스 포스 기준으로 일정 반경 안 난수
	
	FVector patrolPos;

	// 반경
	int radius = 450;
	
	// 
	int symbol;
	int pos;

	// X
	symbol = rand() % 2;
	pos = rand() % radius;
	patrolPos._x = _basePos._x + ((symbol == 0) ? pos * -1 : pos);
	
	// Y
	symbol = rand() % 2;
	pos = rand() % radius;
	patrolPos._y = _basePos._y + ((symbol == 0) ? pos * -1 : pos);
	
	// Z
	patrolPos._z = _info.pos().locationz();

	return patrolPos;
}

bool Monster::GetNextPos(OUT PROTOCOL::ObjectInfo& nextPos)
{
	//
	if (_ownerRoom == nullptr)
		return false;

	// 타겟할 위치
	float targetX;
	float targetY;

	// 리턴, 추적, 순찰
	// 리턴일 때
	if (_currentState->GetType() == MONSTER_STATE::RETURN) {
		targetX = _basePos._x;
		targetY = _basePos._y;
	}

	// 추적일 때
	else if (_currentState->GetType() == MONSTER_STATE::CHASE) {
		shared_ptr<Player> target = _target.lock();
		if (target == nullptr) 
			return false;

		targetX = target->_info.pos().locationx();
		targetY = target->_info.pos().locationy();
	}

	// 순찰
	else {
		targetX = _patrolPos._x;
		targetY = _patrolPos._y;
	}

	// 계산
	// 벡터 x, y
	float dx = targetX - _info.pos().locationx();
	float dy = targetY - _info.pos().locationy();
	
	// 벡터 크기
	float distance = _ownerRoom->DistanceToTarget(dx, dy);

	// 방향 * 스피드 = 각 방향 최대 이동거리
	float x = (dx / distance) * _info.stat().speed() * _deltaTime;
	float y = (dy / distance) * _info.stat().speed() * _deltaTime;

	// 목적지가 최대 이동 거리보다 짧을 때
	if (abs(x) > abs(dx)) x = dx;
	if (abs(y) > abs(dy)) y = dy;

	// 산출된 다음 이동 위치
	nextPos.mutable_pos()->set_locationx(_info.pos().locationx() + x);
	nextPos.mutable_pos()->set_locationy(_info.pos().locationy() + y);
	nextPos.mutable_pos()->set_locationz(_info.pos().locationz());

	return true;
}



/*-----------------------------------------------------------
	Projectile
-----------------------------------------------------------*/

void Projectile::OnDamaged(shared_ptr<GameObject> attacker, int damage)
{
	GameObject::OnDamaged(attacker, damage);
}

void Projectile::OnDead(shared_ptr<GameObject> attacker)
{
	GameObject::OnDead(attacker);
}
