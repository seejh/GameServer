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
	// HP ����, ����� ����
	int hp = _info.stat().hp();
	int realDamage = max(damage - _totalDefence, 0);
	
	hp = max(hp - realDamage, 0);

	_info.mutable_stat()->set_hp(hp);

	// S_ChangeHp ��Ŷ
	PROTOCOL::S_ChangeHp toPkt;
	toPkt.set_object(_info.objectid());
	toPkt.set_hp(hp);
	
	// ��ε�ĳ����
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerRoom->Broadcast(_info.pos().locationx(), _info.pos().locationy(), sendBuffer);

	// DEAD
	if (hp <= 0) 
		OnDead(attacker);
}

void GameObject::OnDead(shared_ptr<GameObject> attacker)
{
	// S_Die ��Ŷ
	PROTOCOL::S_Die toPkt;
	
	// ������ ����
	toPkt.set_attacker(attacker->_info.objectid());
	
	// ������ ����
	toPkt.set_victim(_info.objectid());
	
	// S_Die ��Ŷ ��ε�ĳ����
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerRoom->Broadcast(_info.pos().locationx(), _info.pos().locationy(), sendBuffer);
	
	// �� ����
	_ownerRoom->LeaveRoom(_info.objectid());
}

/*------------------------------------------------------------------------------
	�÷��̾�
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

	// �����ڰ� ���͸� Ÿ�� ����
	if (PROTOCOL::GameObjectType::MONSTER == ObjectManager::Instance()->GetObjectTypeById(attacker->_info.objectid())) {
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(attacker);
		monster->_target.reset();
	}
}

void Player::UseItem(PROTOCOL::C_UseItem fromPkt)
{
	// ������ ����
	shared_ptr<Item> item = _inven.Get(fromPkt.itemdbid());
	if (item == nullptr)
		return;
	
	// �Ҹ�ǰ
	if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
		//
		DBConnection* dbConn = GDBConnectionPool->Pop();

		//
		shared_ptr<Consumable> consumable = static_pointer_cast<Consumable>(item);
		if (consumable == nullptr)
			return;

		// ������ȸ
		ConsumableData* consumableData = reinterpret_cast<ConsumableData*>(DataManager::Instance()->_itemTable[item->_itemInfo.templateid()]);
		if (consumableData != nullptr) {

			// �÷��̾� ü�� ����
			int32 maxhp = _info.stat().maxhp();
			int32 recoveredHp = _info.stat().hp() + consumableData->_recovery;

			_info.mutable_stat()->set_hp(((maxhp < recoveredHp) ? maxhp : recoveredHp));

			// ������ ī��Ʈ - 1
			consumable->_itemInfo.set_count(consumable->_itemInfo.count() - 1);

			// �Ҹ�ǰ ��� ��� -> ����
			if (consumable->_itemInfo.count() <= 0) {
				// DB - ����
				SP::DeleteItem deleteItem(*dbConn);
				deleteItem.In_ItemDbId(consumable->_itemInfo.itemdbid());

				deleteItem.Execute();

				// ���� �޸� ����
				_inven.Remove(consumable->_itemInfo.itemdbid());
			}

			// �Ҹ�ǰ ������ ���� ���� -> ������Ʈ
			else {
				// DB - ����
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

			// �����ۻ�� ��Ŷ
			{
				PROTOCOL::S_UseItem useItemPkt;
				useItemPkt.set_itemdbid(fromPkt.itemdbid());
				useItemPkt.set_use(true);
				
				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(useItemPkt);
				_ownerSession->SendPacket(sendBuffer);
			}

			// HP���� ��Ŷ - ������Ʈ�ѹ�, ü��
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

	// ����
	else {
		//
		DBConnection* dbConn = GDBConnectionPool->Pop();

		// ���� ��û�ε� �ش� ������ �̹� ��� ������ ���� ���� ó��
		if (fromPkt.use() == true) {
			// 
			shared_ptr<Item> unequipItem;

			// �����Ϸ��� ������(item)�� ���� �����ǿ� �����Ǿ� �ִ� �������� �ִ°�
			unequipItem = _inven.FindEquipped(item);
			if (unequipItem != nullptr) {
				
				// ���� �޸�
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
				
				// Ŭ���� (����)
				PROTOCOL::S_UseItem toPktUnUse;
				toPktUnUse.set_itemdbid(unequipItem->_itemInfo.itemdbid());
				toPktUnUse.set_use(false);

				auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPktUnUse);
				_ownerSession->SendPacket(sendBuffer);
			}
		}

		// ��û�� ��� ���� or ���� ó��
		// ���� �޸�
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

		// Ŭ���� (����)
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
	// ���� ���� �������� ���� ��ü ��, �� ������Ʈ
	
	//
	_totalDamage = _info.stat().damage();
	_totalDefence = 0;

	_weaponDamage = 0;
	_armorDefence = 0;

	// ������ ��Ͽ���
	for (auto p : _inven._items) {
		// ������
		shared_ptr<Item> item = p.second;

		// ������ �н�
		if (item->_itemInfo.equipped() == false)
			continue;

		// ����� ���� - ��,�� �ջ�
		if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_WEAPON)
			_weaponDamage += static_pointer_cast<Weapon>(item)->_damage;
		else if (item->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR)
			_armorDefence += static_pointer_cast<Armor>(item)->_defence;
	}

	// ��ü ��,�� ���ݿ� ����� �ջ�
	_totalDamage += _weaponDamage;
	_totalDefence += _armorDefence;
}

void Player::OnLeaveGame()
{
	// TODO : �÷��̾� ���� DB�� ����

	// DB Ŀ�ؼ�
	DBConnection* dbConn = GDBConnectionPool->Pop();

	// ����
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

	// ������ : �������� �ϴ� �ǽð� �ݿ��ǰ� �־ �н�


	// DB Ŀ�ؼ� �ݳ�
	GDBConnectionPool->Push(dbConn);
}

void Player::AddExp(int exp)
{
	// �ش� ������ ������ ����ġ(Max)
	int32 maxExp = DataManager::Instance()->_statTable[_info.stat().level()].totalExp;

	// ����� ����ġ = ���� exp + �߰� exp
	_info.mutable_stat()->set_exp(_info.stat().exp() + exp);
	
	// Exp ��Ŷ
	PROTOCOL::S_AddExp toPkt;
	toPkt.set_exp(_info.stat().exp());

	// Exp ����
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);

	// ������
	if (_info.stat().exp() >= maxExp) {
		
		// ���� ���� ����
		StatData nextStat = DataManager::Instance()->_statTable[_info.stat().level() + 1];
		_info.mutable_stat()->set_level(nextStat.level);
		_info.mutable_stat()->set_maxhp(nextStat.maxhp);
		_info.mutable_stat()->set_hp(nextStat.maxhp);
		_info.mutable_stat()->set_damage(nextStat.damage);
		//_info.mutable_stat()->set_exp();
		_info.mutable_stat()->set_totalexp(nextStat.totalExp);

		// ������ ��Ŷ
		PROTOCOL::S_LevelUp toPkt2;
		toPkt2.mutable_info()->mutable_stat()->CopyFrom(_info.stat());

		// ������ ��Ŷ ����
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt2);
		_ownerSession->SendPacket(sendBuffer);
	}
}


/*--------------------------------------------------------------------
	����
-------------------------------------------------------------------*/

Monster::Monster() : _currentState(StateIdle::Instance())
{
}

void Monster::Update()
{
	// ��Ÿ
	uint64 nowTime = GetTickCount64();
	_deltaTime = (float)(nowTime - _lastUpdateTime) / 1000;
	_lastUpdateTime = nowTime;

	// ���¿� ���� Update
	if (_currentState)
		_currentState->Execute(static_pointer_cast<Monster>(shared_from_this()));
	
	// ���� Update ����, 0.2�� �ֱ�
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

	// ������ == �÷��̾�
	if (attacker->_info.objecttype() == PROTOCOL::GameObjectType::PLAYER) {
		// ����ġ ����
		int exp = DataManager::Instance()->_monsterTable[_templateId].stat.totalexp();
		static_pointer_cast<Player>(attacker)->AddExp(exp);

		// ������ ����
		RewardData reward = GetRandomReward();
		if (reward.itemId != 0) 
			DBTransaction::Instance()->DoAsync(&DBTransaction::RewardPlayer, static_pointer_cast<Player>(attacker), reward, attacker->_ownerRoom);
	}

	// ������ ����
	_info.mutable_pos()->set_locationx(_basePos._x);
	_info.mutable_pos()->set_locationy(_basePos._y);
	_info.mutable_pos()->set_locationz(_basePos._z);

	_info.mutable_stat()->set_hp(_info.stat().maxhp());
	_currentState = StateIdle::Instance();

	room->DoTimer(10000, &Room::EnterRoom, shared_from_this());
}

void Monster::Init(int templateId)
{
	// ���� ���� ��ȸ
	if (DataManager::Instance()->_monsterTable.count(templateId) == 0) {
		cout << "Monster-" << _info.objectid() << ", Init Error : No MonsterTable" << endl;
		return;
	}

	MonsterData data = DataManager::Instance()->_monsterTable[templateId];
	
	// ���� ���� ����
	_templateId = data.id;
	_info.set_objecttype(PROTOCOL::GameObjectType::MONSTER);
	_info.set_name(data.name);
	_info.mutable_stat()->CopyFrom(data.stat);
	_totalDamage = _info.stat().damage();

	// ���� ���� ����
	_currentState = StateIdle::Instance();

	// 
	_lastUpdateTime = GetTickCount64();
}

void Monster::ChangeState(State* state)
{
	// ������ EXIT
	_currentState->Exit(static_pointer_cast<Monster>(shared_from_this()));

	// ���� ����
	_currentState = state;

	// ����� ���� Enter
	_currentState->Enter(static_pointer_cast<Monster>(shared_from_this()));
}

void Monster::Skill(uint64 nowTime)
{
	// Ÿ�� ����
	shared_ptr<Player> target = _target.lock();
	if (target != nullptr && _ownerRoom != nullptr) {
		// S_Skill ��Ŷ
		PROTOCOL::S_Skill toPkt;

		// ������ ����
		toPkt.add_victims(target->_info.objectid());
		
		// ������ ����
		toPkt.set_attacker(_info.objectid());
		
		// ��ų ����
		toPkt.set_skillid(PROTOCOL::SkillType::SKILL_AUTO);

		// ��ų �� ���
		_nextAttackTime = nowTime + _info.stat().attackcooltime();

		// S_Skill ��Ŷ ��ε�ĳ����
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerRoom->Broadcast(_info.pos().locationx(), _info.pos().locationy(), sendBuffer);

		// �ǰ� ������Ʈ
		// ��ų ��ȸ
		auto p = DataManager::Instance()->_skillTable.find(PROTOCOL::SkillType::SKILL_AUTO);
		if (p == DataManager::Instance()->_skillTable.end())
			return;

		// ����� ��� (��ų �� + ����) 
		int damage = p->second.damage + _totalDamage;
		
		// �ǰ� ����
		target->OnDamaged(static_pointer_cast<Monster>(shared_from_this()), damage);
	}
}

RewardData Monster::GetRandomReward()
{
	// Ȯ��
	int ran = rand() % 100;

	// ���� ���� ���
	MonsterData monsterData = DataManager::Instance()->_monsterTable[_templateId];
	for (RewardData reward : monsterData.rewardDatas) {
		
		// ���
		if (ran <= reward.itemDropRate) 
			return reward;
	}

	// ���X
	return RewardData();
}

FVector Monster::GetRandomPatrolPos()
{
	// ���̽� ���� �������� ���� �ݰ� �� ����
	
	FVector patrolPos;

	// �ݰ�
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

	// Ÿ���� ��ġ
	float targetX;
	float targetY;

	// ����, ����, ����
	// ������ ��
	if (_currentState->GetType() == MONSTER_STATE::RETURN) {
		targetX = _basePos._x;
		targetY = _basePos._y;
	}

	// ������ ��
	else if (_currentState->GetType() == MONSTER_STATE::CHASE) {
		shared_ptr<Player> target = _target.lock();
		if (target == nullptr) 
			return false;

		targetX = target->_info.pos().locationx();
		targetY = target->_info.pos().locationy();
	}

	// ����
	else {
		targetX = _patrolPos._x;
		targetY = _patrolPos._y;
	}

	// ���
	// ���� x, y
	float dx = targetX - _info.pos().locationx();
	float dy = targetY - _info.pos().locationy();
	
	// ���� ũ��
	float distance = _ownerRoom->DistanceToTarget(dx, dy);

	// ���� * ���ǵ� = �� ���� �ִ� �̵��Ÿ�
	float x = (dx / distance) * _info.stat().speed() * _deltaTime;
	float y = (dy / distance) * _info.stat().speed() * _deltaTime;

	// �������� �ִ� �̵� �Ÿ����� ª�� ��
	if (abs(x) > abs(dx)) x = dx;
	if (abs(y) > abs(dy)) y = dy;

	// ����� ���� �̵� ��ġ
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
