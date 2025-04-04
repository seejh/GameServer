#pragma once

#include"DataContents.h"
#include"Inventory.h"
#include"VisionCube.h"
#include"DBDataModel.h"

// 추후 제거 필요

enum {
	OBJECT_JOB_SIZE = 20,
};

enum PLAYERJOBS {
	PLAYER_JOB_NONE, PLAYER_JOB_VISION, PLAYER_JOB_SAVE, 
};

enum MONSTERJOBS {
	MONSTER_JOB_NONE, MONSTER_JOB_UPDATE
};

class State;
class GameRoom;
class GameSession;
class GameObject : public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage);
	virtual void OnDead(shared_ptr<GameObject> attacker);
	virtual void OnLeaveGame();
	virtual void OnEnterGame();

public:
	PROTOCOL::ObjectInfo _info; // pos, stat
	
	int _totalDamage = 0;
	int _totalDefence = 0;

	// weak_ptr<Job> _reservedJob;
	vector<weak_ptr<Job>> _reservedJobs;
	shared_ptr<GameRoom> _ownerRoom;
};

class Player : public GameObject {
public:
	Player();
	virtual ~Player();

	// override
	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage) override;
	virtual void OnDead(shared_ptr<GameObject> attacker) override;
	virtual void OnLeaveGame() override;
	virtual void OnEnterGame() override;
 
	Inventory& GetInven() { return _inven; }

	// 
	void UseItem(int itemId);
	// void TakeReward(int itemId, int quantity);
	void CalculateAddStat();
	void AddExp(int exp);
	ItemDB CanAddItemDB(int itemId, int count);
	void AddItem(ItemDB itemDB);
	void CompleteQuest(QuestDB questDB);
	void RemoveQuest(QuestDB questDB);
	void AddQuest(QuestDB questDB);
	void UpdateQuest(QuestType questType, int objectiveId, int quantity);

	// 플레이어 종료 시 메모리에 내용을 DB에 저장 (스텟, 아이템X, 퀘스트)
	void SaveToDB(bool init);

public:
	int _playerDbId;

	int _weaponDamage = 0;
	int _armorDefence = 0;
	
	Inventory _inven;
	VisionCube _vision;
	shared_ptr<class QuestManager> _questManager;

	queue<ItemDB> _inventoryUpdateQueue;
	shared_ptr<GameSession> _ownerSession;

	// TEST
	bool _isBot = false;

	float _testX;
	float _testY;
};

class Monster : public GameObject {
public:
	Monster();

	// override
	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage) override;
	virtual void OnDead(shared_ptr<GameObject> attacker) override;
	virtual void OnLeaveGame() override;
	virtual void OnEnterGame() override;

	// 
	void Init(int templateId);
	void Update();
	void ChangeState(State* state);
	void Skill();
	
	RewardData GetRandomReward();
	PROTOCOL::PFVector GetRandomPatrolPos();
	
	// bool GetNextPos(OUT PROTOCOL::ObjectInfo& nextPos);
public:
	State* _currentState;
	weak_ptr<Player> _target;

	uint64 _lastUpdateTime = 0;
	uint64 _nowUpdateTime = 0;
	uint64 _moveDelta = 0;
	
	uint64 _nextAttackTime = 0;
	uint64 _nextPatrolTime = 0;
	bool _isPatrol = false;

	PROTOCOL::PFVector _basePos;
	PROTOCOL::PFVector _patrolPos;
};

// 
class Npc : public GameObject {
public:
	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage) override;
	virtual void OnDead(shared_ptr<GameObject> attacker) override;
	virtual void OnLeaveGame() override;
	virtual void OnEnterGame() override;

	void Init(int templateId);
	// void Update();

public:
	
};

// 아이템