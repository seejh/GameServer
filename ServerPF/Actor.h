#pragma once

class ClientSession;

enum ActorType { PLAYER, MONSTER };

extern atomic<uint64> g_playerIndexGenerator;
extern atomic<uint64> g_monsterIndexGenerator;

/*-----------------------------------------------------------------------
	FVector
-----------------------------------------------------------------------*/
class FVector {
public:
	FVector();
	FVector(float x, float y, float z);

	void Set(float x, float y, float z);
public:
	float _x;
	float _y;
	float _z;
};


/*-----------------------------------------------------------------------
	FRotator
-----------------------------------------------------------------------*/
class FRotator {
public:
	FRotator();
	FRotator(float yaw, float pitch, float roll);

	void Set(float yaw, float pitch, float roll);
public:
	float _yaw;
	float _pitch;
	float _roll;
};



/*-----------------------------------------------------------------------
	Actor
------------------------------------------------------------------------*/
class Actor : enable_shared_from_this<Actor> 
{
public:
	Actor(ActorType type) : _actorType(type), _index(-1), _location(0, 0, 0), _rotation(0, 0, 0), _velocity(0, 0, 0), _hp(100), _damage(2), _alive(false) {

	}
	virtual ~Actor() {}

	/*----------------------------------------------------------------------
		Getter
	----------------------------------------------------------------------*/
	ActorType GetActorType();
	uint64 GetIndex();
	FVector& GetLocation();
	FRotator& GetRotation();
	FVector& GetVelocity();
	int GetHP();
	int GetDamage();
	void SetAlive(bool flag);
	/*----------------------------------------------------------------------
		Setter
	----------------------------------------------------------------------*/
	void SetIndex(uint64 index);
	void SetLocation(FVector location);
	void SetLocation(float x, float y, float z);
	void SetRotation(FRotator rotation);
	void SetRotation(float yaw, float pitch, float roll);
	void SetVelocity(FVector velocity);
	void SetVelocity(float x, float y, float z);
	void SetHP(int hp);
	void SetDamage(int damage);
	bool IsAlive();
public:
	ActorType _actorType;
	uint64 _index;
	FVector _location;
	FRotator _rotation;
	FVector _velocity;
	int _hp;
	int _damage;
	bool _alive;
};

class Player : public Actor {
public:
	Player() : Actor(ActorType::PLAYER), _isLogin(false) {}

	bool IsLogin();
	void SetLogin(bool flag);
public:
	bool _isLogin;
	shared_ptr<ClientSession> _ownerSession;
};


enum MONSTER_STATE { IDLE, MOVE, ATTACK };
class Monster : public Actor {
public:
	Monster() : Actor(ActorType::MONSTER) {}

	void Update(float deltaTime);
private:
	void UpdateIdle(float deltaTime);
	void UpdateMove(float deltaTime);
	void UpdateAttack(float deltaTime);
	//void UpdateDead(float deltaTime);

	bool IsPlayerAround();
	bool IsAttackRangeIn();
	bool IsAttackTime();

	void LookAt();
	void Attack(float deltaTime);
	void Move(float deltaTime);

public:
	FVector _originLocation;

	float _attackDistance = 150.0f;
	float _noticeDistance = 1000.0f;
	float _returnDistance = 2000.0f;
	float _speed = 300.0f;
	float _attackCoolTime = 0;

	uint64 _lastUpdateTime = -1;

	MONSTER_STATE _state = MONSTER_STATE::IDLE;

	shared_ptr<Player> _target;
};

