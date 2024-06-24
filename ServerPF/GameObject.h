#pragma once

#include"DataContents.h"
#include"Inventory.h"
#include"VisionCube.h"


class FVector {
public:
	float _x;
	float _y;
	float _z;
};


class State;
class Room;
class ClientSession;
class GameObject : public enable_shared_from_this<GameObject>
{
public:
	virtual ~GameObject();

	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage);
	virtual void OnDead(shared_ptr<GameObject> attacker);

public:
	PROTOCOL::ObjectInfo _info; // pos, stat
	
	int _totalDamage = 0;
	int _totalDefence = 0;

	weak_ptr<Job> _reservedJob;
	shared_ptr<Room> _ownerRoom;
};

class Player : public GameObject {
public:
	Player();
	virtual ~Player();

	// override
	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage) override;
	virtual void OnDead(shared_ptr<GameObject> attacker) override;

	// 
	void UseItem(PROTOCOL::C_UseItem fromPkt); // TODO : 家葛前 贸府
	void CalculateAddStat();
	void OnLeaveGame();
	void AddExp(int exp);
public:
	int _playerDbId;

	int _weaponDamage = 0;
	int _armorDefence = 0;
	
	Inventory _inven;
	VisionCube _vision;

	shared_ptr<ClientSession> _ownerSession;
};

class Monster : public GameObject {
public:
	Monster();

	// override
	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage) override;
	virtual void OnDead(shared_ptr<GameObject> attacker) override;

	// 
	void Init(int templateId);
	void Update();
	void ChangeState(State* state);
	void Skill(uint64 nowTime);
	
	RewardData GetRandomReward();
	FVector GetRandomPatrolPos();
	bool GetNextPos(OUT PROTOCOL::ObjectInfo& nextPos);
public:
	int _templateId;
	State* _currentState;
	weak_ptr<Player> _target;

	uint64 _lastUpdateTime = 0;
	float _deltaTime = 0;
	uint64 _nextAttackTime = 0;
	uint64 _nextPatrolTime = 0;
	bool _isPatrol = false;

	FVector _basePos;
	FVector _patrolPos;
};

class Projectile : public GameObject {
public:
	// override
	virtual void OnDamaged(shared_ptr<GameObject> attacker, int damage) override;
	virtual void OnDead(shared_ptr<GameObject> attacker) override;

	//
public:

};