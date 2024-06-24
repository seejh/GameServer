#include "pch.h"
#include "Actor.h"

#include"Protocol3.pb.h"
#include"ClientPacketHandler.h"
#include"Room.h"

atomic<uint64> g_playerIndexGenerator = 1;
atomic<uint64> g_monsterIndexGenerator = 1;

/*-----------------------------------------------------------------------
	FVector
-----------------------------------------------------------------------*/
FVector::FVector() : _x(0.f), _y(0.f), _z(0.f) {}
FVector::FVector(float x, float y, float z) : _x(x), _y(y), _z(z) {}

void FVector::Set(float x, float y, float z) { _x = x; _y = y; _z = z; }

/*-----------------------------------------------------------------------
	FRotator
-----------------------------------------------------------------------*/
FRotator::FRotator() : _yaw(0.f), _pitch(0.f), _roll(0.f) {}
FRotator::FRotator(float yaw, float pitch, float roll) : _yaw(yaw), _pitch(pitch), _roll(roll) {}

void FRotator::Set(float yaw, float pitch, float roll) { _yaw = yaw; _pitch = pitch; _roll = roll; }

/*
	Actor
*/
ActorType Actor::GetActorType() { return _actorType; }
uint64 Actor::GetIndex() { return _index; }
FVector& Actor::GetLocation() { return _location; }
FRotator& Actor::GetRotation() { return _rotation; }
FVector& Actor::GetVelocity() { return _velocity; }
int Actor::GetHP() { return _hp; }
int Actor::GetDamage() { return _damage; }
void Actor::SetAlive(bool flag) { _alive = flag; }

void Actor::SetIndex(uint64 index) { _index = index; }
void Actor::SetLocation(FVector location) { _location = location; }
void Actor::SetLocation(float x, float y, float z) { _location._x = x; _location._y = y; _location._z = z; }
void Actor::SetRotation(FRotator rotation) { _rotation = rotation; }
void Actor::SetRotation(float yaw, float pitch, float roll) { _rotation._yaw = yaw; _rotation._pitch = pitch; _rotation._roll = roll; }
void Actor::SetVelocity(FVector velocity) { _velocity = velocity; }
void Actor::SetVelocity(float x, float y, float z) { _velocity._x = x; _velocity._y = y; _velocity._z = z; }
void Actor::SetHP(int hp) { _hp = hp; }
void Actor::SetDamage(int damage) { _damage = damage; }
bool Actor::IsAlive() { return _alive;}

/*------------------------------------------------------------------------
	Player
--------------------------------------------------------------------------*/
bool Player::IsLogin() { return _isLogin; }
void Player::SetLogin(bool flag) { _isLogin = flag; }

/*----------------------------------------------------------------------
	Monster
-------------------------------------------------------------------------*/

void Monster::Update(float deltaTime)
{
	// 0이하 쿨타임 끝
	if (_attackCoolTime >= 0)
		_attackCoolTime -= deltaTime;

	switch (_state) {
	case MONSTER_STATE::IDLE:
		UpdateIdle(deltaTime);
		break;
	case MONSTER_STATE::MOVE:
		UpdateMove(deltaTime);
		break;
	case MONSTER_STATE::ATTACK:
		UpdateAttack(deltaTime);
		break;
	default:
		break;
	}
}

void Monster::UpdateIdle(float deltaTime)
{
	// 주변에 플레이어가 있나
	if (IsPlayerAround()) {
		_state = MONSTER_STATE::MOVE;
	}
	// 주변에 없음
	else {
		// TODO : patrol
	}
}

void Monster::UpdateMove(float deltaTime)
{
	// 타겟이 유효하고, 로그인 되어 있고, 살아 있다면
	if (_target && _target->IsLogin() && _target->IsAlive()) {
		// 공격할 수 있는 거리인가
		if (IsAttackRangeIn()) {
			// 공격 쿨이 돌았나
			if (IsAttackTime()) {
				_state = MONSTER_STATE::ATTACK;
			}
			// 공격 쿨이 안 돌았으면 그냥 대기, nothing to do
		}
		// 공격할 수 없는 거리라면
		else {
			Move(deltaTime);
		}
	}
}

void Monster::UpdateAttack(float deltaTime) {
	// 타겟이 유효하다면
	if (_target && _target->IsLogin() && _target->IsAlive()) {
		// 공격할 수 있는 거리인가
		if (IsAttackRangeIn()) {
			// 공격 쿨이 돌았나
			if (IsAttackTime()) {
				Attack(deltaTime);
			}
			// 공격 쿨 안돔 대기 nothing
		}

		// 공격 범위 밖
		else {
			_state = MONSTER_STATE::MOVE;
		}
	}

	// 타겟이 유효하지 않음
	else {

	}
}

bool Monster::IsPlayerAround()
{
	map<uint64, shared_ptr<Player>>::iterator it;
	for (it = Room::Instance()->_players.begin(); it != Room::Instance()->_players.end(); it++) {
		shared_ptr<Player> player = it->second;

		// 좌, 우, 아래, 위
		if (GetLocation()._x - _noticeDistance <= player->GetLocation()._x &&
			player->GetLocation()._x <= GetLocation()._x + _noticeDistance &&
			GetLocation()._y - _noticeDistance <= player->GetLocation()._y &&
			player->GetLocation()._y <= GetLocation()._y + _noticeDistance) {

			_target = player;
			return true;
		}
	}

	return false;
}

// -10 -6 -2 2 6 10     // 4
bool Monster::IsAttackRangeIn() {
	// 나와 타깃의 x,y차이가 공격 사거리 이하일 때
	if (abs(GetLocation()._x - _target->GetLocation()._x) <= _attackDistance &&
		abs(GetLocation()._y - _target->GetLocation()._y) <= _attackDistance) {
		return true;
	}

	return false;
}

bool Monster::IsAttackTime() {
	if (_attackCoolTime <= 0)
		return true;

	return false;
}

void Monster::Attack(float deltaTime) {
	// 타겟 피격 후 체력 업데이트
	_target->SetHP(_target->GetHP() - GetDamage());

	//LookAt();

	// 공격 후 maxcooltime 적용
	_attackCoolTime = 1.2f;

	// 브로드 캐스트
	PROTOCOL::S_ATTACK pkt;
	pkt.mutable_attacker()->set_actortype(PROTOCOL::MONSTER);
	pkt.mutable_attacker()->set_index(GetIndex());
	pkt.mutable_attacker()->set_rotationyaw(GetRotation()._yaw);

	PROTOCOL::ACTOR* victim = pkt.add_victims();
	victim->set_actortype(PROTOCOL::PLAYER);
	victim->set_index(_target->GetIndex());
	victim->set_hp(_target->GetHP());

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Room::Instance()->BroadcastWithoutLock(sendBuffer);
}

void Monster::Move(float deltaTime) {
	// 이동 가능 최대 거리
	float maxDistanceEachTime = _speed * deltaTime;
	FVector pos = GetLocation();

	// Xpos = 나와 플레이어의 x상 거리에서 공격 사거리를 뺀 거리와 이동 가능 최대 거리 중 더 작은 값
	float moveDistanceX = min(abs(pos._x - _target->GetLocation()._x) - _attackDistance, maxDistanceEachTime);

	// 내가 타깃의 x상 좌측에 위치
	if (pos._x <= _target->GetLocation()._x)
		pos._x += moveDistanceX;

	// 내가 타깃의 x상 우측에 위치
	else
		pos._x -= moveDistanceX;

	// Ypos = 위의 내용에서 y상 거리
	float moveDistanceY = min(abs(pos._y - _target->GetLocation()._y) - _attackDistance, maxDistanceEachTime);

	// 내가 타깃의 y상 밑에 위치
	if (pos._y <= _target->GetLocation()._y)
		pos._y += moveDistanceY;

	// 내가 타깃의 y상 위에 위치
	else
		pos._y -= moveDistanceY;

	// 계산한 값을 좌표로 설정
	SetLocation(pos);

	// 브로드 캐스팅
	PROTOCOL::S_MOVE pkt;
	PROTOCOL::ACTOR* protoMonster = pkt.mutable_actor();
	protoMonster->set_index(GetIndex());
	protoMonster->set_actortype(PROTOCOL::MONSTER);
	protoMonster->set_locationx(pos._x);
	protoMonster->set_locationy(pos._y);
	protoMonster->set_locationz(pos._z);
	
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
	Room::Instance()->BroadcastWithoutLock(sendBuffer);
}

void Monster::LookAt()
{
	float dirX = _target->GetLocation()._x - GetLocation()._x;
	float dirY = _target->GetLocation()._y - GetLocation()._y;
	double target_yaw = atan2(dirY, dirX);
	double yawInDegrees = target_yaw * 180.0 / 3.14;

	_rotation._yaw = target_yaw;
}

