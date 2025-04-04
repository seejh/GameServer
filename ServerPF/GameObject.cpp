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
	// HP ����, ����� ����
	int hp = _info.stat().hp();
	int realDamage = max(damage - _totalDefence, 0);
	
	hp = max(hp - realDamage, 0);

	_info.mutable_stat()->set_hp(hp);

	// S_ChangeHp ��Ŷ
	PROTOCOL::S_CHANGE_HP toPkt;
	toPkt.set_object(_info.objectid());
	toPkt.set_hp(hp);
	
	// ���� ��ε�ĳ����
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
	// S_Die ��Ŷ (������, ������ ����) ���� ��ε�ĳ����
	PROTOCOL::S_DIE toPkt;
	toPkt.set_attacker(attacker->_info.objectid());
	toPkt.set_victim(_info.objectid());
	shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerRoom->Broadcast(_info.pos().location().x(), _info.pos().location().y(), sendBuffer);
	
	// �� ����
	_ownerRoom->LeaveRoom(_info.objectid());
}

void GameObject::OnLeaveGame()
{
	// TODO : 
	// Die�� �״� ��� ����� ���� �ʿ��� ��Ŷ�̸�
	// ���� ���� ��ü�� �뿡�� �����ߴٴ� ���� LeaveRoom ��Ŷ���� ó���ؾ��Ѵ�.

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

	// ���� �ƴ϶� ���� �÷��̾�� DB�۾�
	if (_isBot == false) {
		DBConnection* dbConn = DBManager::Instance()->_gameDbManager->Pop();

		// ����
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
			if (updateQuest.Execute() == false) 
				cout << "[PLAYER-" << _info.objectid() << "] OnLeaveRoom Error - UpdateQuest Execute Failed" << endl;
		}

		DBManager::Instance()->_gameDbManager->Push(dbConn);
	}

	// ��Ŷ
	PROTOCOL::S_LEAVE_ROOM toPkt;
	toPkt.mutable_object()->set_objectid(_info.objectid());
	toPkt.mutable_object()->set_objecttype(_info.objecttype());
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	_ownerSession->SendPacket(sendBuffer);

	// �÷��̾� ���� ��Ȱ��ȭ
	_vision._prevObjects.clear();
	_vision._ownerPlayer.reset();
	
	// �� ���
	_ownerRoom.reset();

	// ���ǿ��� �÷��̾� ���� ����
	// _ownerSession->_player.reset();
}

void Player::OnEnterGame()
{
	GameObject::OnEnterGame();

	// ������ ������� ���� ���
	CalculateAddStat();

	// ����ڿ��Ը� S_EnterRoom
	PROTOCOL::S_ENTER_ROOM toPkt;
	toPkt.set_success(true);
	toPkt.mutable_object()->CopyFrom(_info);
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
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
		PROTOCOL::S_CHANGE_HP toPkt;
		toPkt.set_object(_info.objectid());
		toPkt.set_hp(afterHP);

		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
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
		if (item == nullptr) {
			cout << "Player::CalculateAddStat() Error - Item Nullptr Slot : " << p.first << endl;
			continue;
		}

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
	PROTOCOL::S_ADD_EXP toPkt;
	toPkt.set_exp(_info.stat().exp());

	// Exp ����
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
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
		PROTOCOL::S_LEVEL_UP toPkt2;
		toPkt2.mutable_info()->mutable_stat()->CopyFrom(_info.stat());

		// ������ ��Ŷ ����
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt2);
		_ownerSession->SendPacket(sendBuffer);
	}
}

ItemDB Player::CanAddItemDB(int itemId, int count)
{
	// ������Ʈ, �μ�Ʈ / ����
	ItemDB itemDB;
	
	// �Һ��
	if (DataManager::Instance()->_itemTable[itemId]->_itemType == PROTOCOL::ITEM_TYPE_CONSUMABLE) {
		itemDB.Slot = _inven.GetStackPos(itemId);
		// ���� �Ұ���
		if (itemDB.Slot == -1) {
			// �� ���� Ȯ��
			itemDB.Slot = _inven.GetEmptySlot();
			if (itemDB.Slot != -1) {
				itemDB.TemplateId = itemId;
				itemDB.Count = count;
				// itemDB.Slot = ;
				itemDB.Equipped = false;
				itemDB.PlayerDbId = _info.playerdbid();
				itemDB.dbState = DB_STATE::INSERT_NEED;

				if (_inven.SetSlotDBLock(itemDB.Slot, true) == false) {
					// ���� ���� ��, �۾� ���
					itemDB.Slot = -1;
				}
			}
		}
		// ���� ����
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
				// ���� ���� ��, �۾� ���
				itemDB.Slot = -1;
			}
		}
	}

	// ����
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
				// ���� ���� ��, �۾� ���
				itemDB.Slot = -1;
			}
		}
	}

	return itemDB;
}

void Player::AddItem(ItemDB itemDB)
{
	// ����
	if (itemDB.dbState == DB_STATE::INSERT_SUCCESS) {
		// ������ ����
		shared_ptr<Item> item = Item::MakeItem(itemDB);
		if (item != nullptr) {
			_inven.Add(item);

			// ���� ��Ŷ
			PROTOCOL::S_ADD_ITEM toPkt;
			toPkt.add_items()->CopyFrom(item->_itemInfo);
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
			_ownerSession->SendPacket(sendBuffer);
		}
	}
	// ������Ʈ
	else if (itemDB.dbState == DB_STATE::UPDATE_SUCCESS) {
		// ������ ������Ʈ
		shared_ptr<Item>& item = _inven._items[itemDB.Slot];
		if (item != nullptr) {
			item->_itemInfo.set_itemdbid(itemDB.ItemDbId);
			item->_itemInfo.set_playerdbid(itemDB.PlayerDbId);
			item->_itemInfo.set_templateid(itemDB.TemplateId);
			item->_itemInfo.set_count(itemDB.Count);
			item->_itemInfo.set_slot(itemDB.Slot);
			item->_itemInfo.set_equipped(itemDB.Equipped);

			// ������Ʈ ��Ŷ
			PROTOCOL::S_UPDATE_ITEM toPkt;
			toPkt.add_items()->CopyFrom(item->_itemInfo);
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
			_ownerSession->SendPacket(sendBuffer);
		}
	}
	// 
	else {

	}

	// ���� �� ����
	_inven.SetSlotDBLock(itemDB.Slot, false);
}

void Player::CompleteQuest(QuestDB questDB)
{
	if (questDB.dbState != DB_STATE::UPDATE_SUCCESS)
		return;

	auto it = _questManager->_quests.find(questDB.TemplateId);
	if (it != _questManager->_quests.end()) {
		// ������Ʈ
		it->second->_questInfo.set_questdbid(questDB.QuestDbId);
		it->second->_questInfo.set_playerdbid(questDB.PlayerDbId);
		it->second->_questInfo.set_templateid(questDB.TemplateId);
		it->second->_questInfo.set_progress(questDB.Progress);
		it->second->_questInfo.set_completed(questDB.Completed);

		// ��Ŷ
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
		// ����
		_questManager->Remove(questDB.TemplateId);

		// ��Ŷ
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

	// ����
	shared_ptr<Quest> quest = Quest::MakeQuest(questDB);
	_questManager->Add(quest);

	// ��Ŷ
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

		// (���� �޸�) ������Ʈ
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

		// ��Ŷ
		PROTOCOL::S_UPDATE_QUEST toPkt;
		toPkt.mutable_questinfo()->CopyFrom(p.second->_questInfo);
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerSession->SendPacket(sendBuffer);
	}
}

void Player::SaveToDB(bool init)
{
	// �׽�Ʈ : �ϴ� ���� 30�� ����
	uint64 saveTick = 3 * 10'000;
	
	// SAVE
	if (init == false) {
		// �÷��̾� ����(����, ��ġ)
		
		
		// �κ��丮

	}

	// ����
	_reservedJobs[PLAYERJOBS::PLAYER_JOB_SAVE] = _ownerRoom->DoTimer(saveTick, [this]() { SaveToDB(false); });
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
	/*uint64 nowTime = GetTickCount64();
	_deltaTime = (float)(nowTime - _lastUpdateTime) / 1000;
	_lastUpdateTime = nowTime;*/

	// 
	_lastUpdateTime = _nowUpdateTime;
	_nowUpdateTime = GetTickCount64();

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

	shared_ptr<GameRoom> room = _ownerRoom;
	
	GameObject::OnDead(attacker);

	// ������ == �÷��̾�
	if (attacker->_info.objecttype() == PROTOCOL::GameObjectType::PLAYER) {

		// ���Ͻ� �н� - TEST
		if (static_pointer_cast<Player>(attacker)->_isBot == false) {

			// ���� ������ ��ȸ
			MonsterData* monsterData = DataManager::Instance()->_monsterTable[_info.typetemplateid()];
			if (monsterData == nullptr)
				return;

			// ����ġ ����
			static_pointer_cast<Player>(attacker)->AddExp(monsterData->stat.totalexp());

			// ������ ����
			RewardData reward = GetRandomReward();
			if (reward.itemId != 0) {
				// static_pointer_cast<Player>(attacker)->TakeReward(reward.itemId, reward.count);

				ItemDB itemDB = static_pointer_cast<Player>(attacker)->CanAddItemDB(reward.itemId, reward.count);
				if (itemDB.Slot != -1) 
					if (static_pointer_cast<Player>(attacker)->_inven.SetSlotDBLock(itemDB.Slot, true) == true)
						DBManager::Instance()->_gameDbManager->DoAsync(&GameDBManager::Transact_AddItem, static_pointer_cast<Player>(attacker), itemDB);
			}
			
			// ����Ʈ ������Ʈ
			static_pointer_cast<Player>(attacker)->UpdateQuest(QuestType::QUEST_TYPE_KILL, _info.typetemplateid(), 1);
		}
	}

	// ������ ����
	_info.mutable_pos()->mutable_location()->CopyFrom(_basePos);

	_info.mutable_stat()->set_hp(_info.stat().maxhp());
	_currentState = StateIdle::Instance();

	room->DoTimer(10000, &GameRoom::EnterRoom, shared_from_this());
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

void Monster::Skill()
{
	// Ÿ�� ����
	shared_ptr<Player> target = _target.lock();
	if (target != nullptr && _ownerRoom != nullptr) {
		// S_Skill ��Ŷ
		PROTOCOL::S_SKILL toPkt;

		// ������ ����
		toPkt.add_victims(target->_info.objectid());
		
		// ������ ����
		toPkt.set_attacker(_info.objectid());
		
		// ��ų ����
		toPkt.set_skillid(PROTOCOL::SkillType::SKILL_AUTO);

		// ��ų �� ���
		_nextAttackTime = _nowUpdateTime + _info.stat().attackcooltime();

		// S_Skill ��Ŷ ���� ��ε�ĳ����
		auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
		_ownerRoom->Broadcast(_info.pos().location().x(), _info.pos().location().y(), sendBuffer);

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

PROTOCOL::PFVector Monster::GetRandomPatrolPos()
{
	// basePos �������� ���� �ݰ� ������ġ
	
	// FVector patrolPos;
	PROTOCOL::PFVector patrolPos;

	// �ݰ�
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
//	// Ÿ���� ��ġ
//	float targetX;
//	float targetY;
//
//	// ����, ����, ����
//	// ������ ��
//	if (_currentState->GetType() == MONSTER_STATE::RETURN) {
//		targetX = _basePos.x();
//		targetY = _basePos.y();
//
//		test = 1;
//	}
//
//	// ������ ��
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
//	// ����
//	else {
//		targetX = _patrolPos.x();
//		targetY = _patrolPos.y();
//
//		test = 3;
//	}
//
//	///////////////////////////////////////////////// ���
//	
//	// �̵� ���� (�� ����)
//	float moveVX = targetX - _info.pos().locationx();
//	float moveVY = targetY - _info.pos().locationy();
//	if (moveVX == 0 && moveVY == 0) {
//		// Ÿ�� ������ ����, �̵� �ʿ� X
//		nextPos.mutable_pos()->set_locationx(_info.pos().locationx());
//		nextPos.mutable_pos()->set_locationy(_info.pos().locationy());
//
//		return true;
//	}
//
//	// �̵� ���� ũ��
//	float dt = _ownerRoom->DistanceToTarget(moveVX, moveVY);
//
//	// ���� ���� (�� ����)
//	float dirX = moveVX / dt;
//	float dirY = moveVY / dt;
//
//	// �ִ� �̵� ���� ��ġ
//	float x = dirX * _info.stat().speed() * _deltaTime;
//	float y = dirY * _info.stat().speed() * _deltaTime;
//
//	// ������ < �ִ� �̵� ���� �Ÿ� -> �������� ��ǥ�� ����
//	if (abs(targetX) < abs(x)) x = targetX;
//	if (abs(targetY) < abs(y)) y = targetY; // �ָ��� �ʿ�
//
//	// �̵� ó��
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

