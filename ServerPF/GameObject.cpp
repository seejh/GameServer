#include "pch.h"
#include "GameObject.h"

#include<random>
#include"DataManager.h"
#include"ObjectManager.h"
#include"Item.h"
#include"ClientPacketHandler.h"
#include"Room.h"
#include"State.h"

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
	// HP ����, ����� ����
	int hp = _info.stat().hp();
	int realDamage = max(damage - _totalDefence, 0);
	
	hp = max(hp - realDamage, 0);

	_info.mutable_stat()->set_hp(hp);

	// S_ChangeHp ��Ŷ
	PROTOCOL::S_ChangeHp toPkt;
	toPkt.set_object(_info.objectid());
	toPkt.set_hp(hp);
	
	// ���� ��ε�ĳ����
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

void GameObject::OnLeaveGame()
{
	// ���� �۾� ��� (���� ����)
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
	�÷��̾�
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

	// �����ڰ� ���͸� Ÿ�� ����
	if (PROTOCOL::GameObjectType::MONSTER == ObjectManager::Instance()->GetObjectTypeById(attacker->_info.objectid())) {
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(attacker);
		monster->_target.reset();
	}
}

void Player::OnLeaveGame()
{
	// ���� �۾�(Job) ���, �÷��̾� ���� DB ����
	GameObject::OnLeaveGame();

	// DB Ŀ�ؼ�
	DBConnection* dbConn = DBManager::Instance()->_gameDbManager->Pop();

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

	// ����Ʈ
	for (auto p : _questManager->_quests) {
		if (p.second->_questInfo.completed())
			continue;

		SP::UpdateQuest updateQuest(*dbConn);
		updateQuest.In_QuestDbId(p.second->_questInfo.questdbid());
		updateQuest.In_TemplateId(p.second->_questInfo.templateid());
		updateQuest.In_PlayerDbId(p.second->_questInfo.playerdbid());
		updateQuest.In_Completed(p.second->_questInfo.completed());
		updateQuest.In_Progress(p.second->_questInfo.progress());
		if (updateQuest.Execute()) {

		}
	}

	// DB Ŀ�ؼ� �ݳ�
	DBManager::Instance()->_gameDbManager->Push(dbConn);

	// ��Ŷ
	PROTOCOL::S_Leave_Room toPkt;
	toPkt.mutable_object()->set_objectid(_info.objectid());
	toPkt.mutable_object()->set_objecttype(_info.objecttype());
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);

	// �÷��̾� ���� ��Ȱ��ȭ
	_vision._prevObjects.clear();
	_vision._ownerPlayer.reset();
	
	// �� ���
	_ownerRoom.reset();

	// �÷��̾� ����
	// _ownerSession->_player.reset();
}

void Player::OnEnterGame()
{
	GameObject::OnEnterGame();

	// ������ ������� ���� ���
	CalculateAddStat();

	// ����ڿ��Ը� S_EnterRoom
	PROTOCOL::S_Enter_Room toPkt;
	toPkt.set_success(true);
	toPkt.mutable_object()->CopyFrom(_info);
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);

	// ���� �÷��̾� Ȱ��ȭ
	_vision._ownerPlayer = static_pointer_cast<Player>(shared_from_this());
	_vision.Update();
}

void Player::UseItem(int itemId)
{
	// ������ ������ ��ȸ�ؼ� �ش� �������� ȿ�� ����

	// ������ ���� ��ȸ
	ItemData* itemData = DataManager::Instance()->_itemTable[itemId];
	if (itemData == nullptr) {
		cout << "Player::UseItem() Error - Can't Find ItemId:" << itemId << "'s itemData" << endl;
		return;
	}
		
	// �Һ���� �ƴϸ� �н�
	if (itemData->_itemType != PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
		cout << "Player::UseItem() Error - ItemType No Consumable" << endl;
		return;
	}
		
	switch (reinterpret_cast<ConsumableData*>(itemData)->_consumableType) {
	case PROTOCOL::ConsumableType::CONSUMABLE_TYPE_HP_POTION: 
	{
		// HP ���Ƿ�
		// ȿ�� ���� - ȸ���� ü�� = ��ü�� + ������ ȸ����
		int32 afterHP = _info.stat().hp() + reinterpret_cast<ConsumableData*>(itemData)->_recovery;
		_info.mutable_stat()->set_hp(afterHP);

		// ��Ŷ
		PROTOCOL::S_ChangeHp toPkt;
		toPkt.set_object(_info.objectid());
		toPkt.set_hp(afterHP);

		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerSession->SendPacket(sendBuffer);
	} 
	break;

	case PROTOCOL::ConsumableType::CONSUMABLE_TYPE_MP_POTION:
		// MP ���Ƿ�
		cout << "Player::UseItem(int) Error - MP POTION" << endl;
		break;

	default:
		// �� ��
		cout << "Player::UseItem(int) Error - Not HP, MP POTION" << endl;
		break;
	}
}

void Player::TakeReward(int itemId, int quantity)
{
	// ������ ��ȸ -> �κ�Ȯ��, DB�۾� ���� -> DB �۾� -> ������ ����(����) -> �κ� �߰� -> ��Ŷ

	// ������ ��ȸ
	ItemData* itemData = DataManager::Instance()->_itemTable[itemId];
	if (itemData == nullptr)
		return;

	// �κ�Ȯ�� �� DB �۾� ����
	ItemDB itemDB;
	int32 slot;

	// �Ҹ�ǰ
	if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {

		// ���ý��� Ȯ�� - ������ UPDATE, ������ �󽽷�Ȯ�� 
		slot = _inven.GetStackPos(itemId);
		if (slot == -1) {
			
			// �󽽷� Ȯ�� - ������ INSERT, ������ NONE
			slot = _inven.GetEmptySlot();
			if (slot == -1) {
				// NONE
			}
			else {
				// INSERT
				
				// ���Զ�
				if (_inven.SetSlotDBLock(slot, true) == false) {
					cout << "Player::TakeReward() Error - CONSUMABLE INSERT, " << slot << "Slot Locked Already" << endl;
					return;
				}

				// DB�۾���û
				itemDB.TemplateId = itemId;
				itemDB.PlayerDbId = _info.playerdbid();
				itemDB.Count = quantity;
				itemDB.Slot = slot;
				itemDB.Equipped = false;
				itemDB.dbState = DB_STATE::INSERT_NEED;

				DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::InsertItem, static_pointer_cast<Player>(shared_from_this()), itemDB, &Room::DBCallback_RewardItem);
			}
		}
		else {
			// UPDATE
			
			// ���Զ�
			if (_inven.SetSlotDBLock(slot, true) == false) {
				cout << "Player::TakeReward() Error - CONSUMABLE UPDATE, " << slot << "Slot Locked Already" << endl;
				return;
			}

			// ������ ��ȸ
			shared_ptr<Item> item = _inven.GetItemBySlot(slot);
			if (item == nullptr) {
				cout << "Player::TakeReward() Error - Find Item Nullptr" << endl;
				return;
			}
				
			// DB�۾���û
			itemDB.ItemDbId = item->_itemInfo.itemdbid();
			itemDB.TemplateId = item->_itemInfo.templateid();
			itemDB.PlayerDbId = item->_itemInfo.playerdbid();
			itemDB.Count = item->_itemInfo.count() + quantity;
			itemDB.Slot = item->_itemInfo.slot();
			itemDB.Equipped = item->_itemInfo.equipped();
			itemDB.dbState = DB_STATE::UPDATE_NEED;

			DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::UpdateItem, static_pointer_cast<Player>(shared_from_this()), itemDB, &Room::DBCallback_RewardItem);
		}
	}

	// ���
	else {
		// �󽽷� Ȯ�� - ������ INSERT, ������ NONE
		slot = _inven.GetEmptySlot();
		if (slot == -1) {
			// NONE
		}
		else {
			// INSERT
			
			// ���Զ�
			if (_inven.SetSlotDBLock(slot, true) == false) {
				cout << "Player::TakeReward() Error - EQUIPMENT INSERT, " << slot << "Slot Locked Already" << endl;
				return;
			}

			// DB�۾���û
			itemDB.TemplateId = itemId;
			itemDB.PlayerDbId = _info.playerdbid();
			itemDB.Count = quantity;
			itemDB.Slot = slot;
			itemDB.Equipped = false;
			itemDB.dbState = DB_STATE::INSERT_NEED;

			DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::InsertItem, static_pointer_cast<Player>(shared_from_this()), itemDB, &Room::DBCallback_RewardItem);
		}
	}
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
	for (auto p : _inven._items) { // ���� ������ ���� �ķ� ���� ��û�޴µ� p��ü�� nullptr -> ��� ���� ���� �ֳ�
		// ������
		shared_ptr<Item> item = p.second;
		if (item == nullptr) 
			cout << "Player::CalculateAddStat() Error - Item Nullptr Slot : " << p.first << endl;

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

void Player::SaveToDB(bool init)
{
	// �׽�Ʈ : �ϴ� ���� 30�� ����
	uint64 SaveTick = 3 * 10'000;
	
	// SAVE
	if (init == false) {
		// �÷��̾� ����(����, ��ġ)
		
		
		// �κ��丮

	}

	// ����
	_reservedJobs[PLAYERJOBS::PLAYER_JOB_SAVE] = _ownerRoom->DoTimer(SaveTick, [this]() { SaveToDB(false); });
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
	_reservedJobs[MONSTERJOBS::MONSTER_JOB_UPDATE] = _ownerRoom->DoTimer(200, [this]() { Update(); });
}

void Monster::OnDamaged(shared_ptr<GameObject> attacker, int damage)
{
	GameObject::OnDamaged(attacker, damage);
}

void Monster::OnDead(shared_ptr<GameObject> attacker)
{
	// ���� �۾� ���, �÷��̾� ����, ������ ����
	GameObject::OnLeaveGame();

	shared_ptr<Room> room = _ownerRoom;
	
	GameObject::OnDead(attacker);

	// ������ == �÷��̾�
	if (attacker->_info.objecttype() == PROTOCOL::GameObjectType::PLAYER) {

		// ���� ������ ��ȸ
		MonsterData* monsterData = DataManager::Instance()->_monsterTable[_info.typetemplateid()];
		if (monsterData == nullptr)
			return;

		// ����ġ ����
		static_pointer_cast<Player>(attacker)->AddExp(monsterData->stat.totalexp());
		
		// ������ ����
		RewardData reward = GetRandomReward();
		if (reward.itemId != 0) 
			static_pointer_cast<Player>(attacker)->TakeReward(reward.itemId, reward.count);
			
		// �÷��̾� ����Ʈ ������Ʈ
		static_pointer_cast<Player>(attacker)->_questManager->Update(QuestType::QUEST_TYPE_KILL, _info.typetemplateid(), 1);
	}

	// ������ ����
	_info.mutable_pos()->set_locationx(_basePos._x);
	_info.mutable_pos()->set_locationy(_basePos._y);
	_info.mutable_pos()->set_locationz(_basePos._z);

	_info.mutable_stat()->set_hp(_info.stat().maxhp());
	_currentState = StateIdle::Instance();

	room->DoTimer(10000, &Room::EnterRoom, shared_from_this());
}

void Monster::OnLeaveGame()
{
	//
	GameObject::OnLeaveGame();

	// ������ �� ���� ����
	_ownerRoom.reset();
}

void Monster::OnEnterGame()
{
	GameObject::OnEnterGame();

	Update();
}

void Monster::Init(int templateId)
{
	// ���� ������ ��ȸ
	MonsterData* data = DataManager::Instance()->_monsterTable[templateId];
	if (data == nullptr) {
		cout << "Monster-" << _info.objectid() << " Init() Error : No MonsterTable" << endl;
		return;
	}

	// ���� ���� ����
	_info.set_typetemplateid(data->id);
	_info.set_objecttype(PROTOCOL::GameObjectType::MONSTER);
	_info.set_name(data->name);
	_info.mutable_stat()->CopyFrom(data->stat);
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

		// S_Skill ��Ŷ ���� ��ε�ĳ����
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
	MonsterData* monsterData = DataManager::Instance()->_monsterTable[_info.typetemplateid()];
	if (monsterData == nullptr) {
		cout << "Monster::GetRandomReward() Error : No MonsterData" << endl;
		return RewardData();
	}

	// �����Ͽ��� ��� Ȯ��
	for (RewardData reward : monsterData->rewardDatas) {
		if (ran <= reward.itemDropRate) 
			return reward;
	}

	// ���X
	return RewardData();
}

FVector Monster::GetRandomPatrolPos()
{
	// basePos �������� ���� �ݰ� ������ġ
	
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

void Projectile::OnLeaveGame()
{
	// GameObject::OnLeaveGame();
}

void Projectile::OnEnterGame()
{
	// GameObject::OnEnterGame();
}

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
	// �ϴ� ��ɾ���
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

