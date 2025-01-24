#include "pch.h"
#include "State.h"

#include"GameObject.h"
#include"Room.h"
#include"ClientPacketHandler.h"

/*-------------------------------------------------------------------
    Idle
-------------------------------------------------------------------*/
StateIdle* StateIdle::Instance()
{
    static StateIdle instance;
    return &instance;
}

MONSTER_STATE StateIdle::GetType()
{
    return MONSTER_STATE::IDLE;
}

void StateIdle::Enter(shared_ptr<Monster> monster)
{
}

void StateIdle::Execute(shared_ptr<Monster> monster)
{
    // 범위 안에 타겟(플레이어)가 있는가
    // 여기서 ownerroom이 nullptr인 경우 - 예상 죽고나서 잡이 취소 되기전 가동
    // 일단...
    if (monster->_ownerRoom == nullptr)
        return;

    float dt = monster->_ownerRoom->GetPlayerAround(monster);

    // 없다 - 순찰 or 대기
    if (dt < 0) {
        
        // 순찰 중 - 순찰 위치에 도달했으면 스톱
        if (monster->_isPatrol) {
            // 순찰 위치 도착 O
            if (monster->_info.pos().locationx() == monster->_patrolPos._x &&
                monster->_info.pos().locationy() == monster->_patrolPos._y) {
               
                // 순찰 해제, 다음 순찰 시간 -> 고쳐야됨
                monster->_isPatrol = false;
                monster->_nextPatrolTime = GetTickCount64() + 7000;
            }

            // 순찰 위치 도착 X
            else {
                
                // 순찰 위치로 계속해서 이동
                PROTOCOL::ObjectInfo info;
                monster->GetNextPos(info);
                monster->_ownerRoom->ActorMove(monster, info);
            }
        }

        // 순찰 아님 - 순찰 설정
        else {
            uint64 nowTime = GetTickCount64();

            // 순찰 쿨이 되었는가
            if (monster->_nextPatrolTime < nowTime) {
                
                // 순찰 위치, 순찰 상태    설정
                monster->_patrolPos = monster->GetRandomPatrolPos();
                
                monster->_isPatrol = true;

                //
                PROTOCOL::ObjectInfo info;
                info.mutable_pos()->set_locationx(monster->_patrolPos._x);
                info.mutable_pos()->set_locationy(monster->_patrolPos._y);
                info.mutable_pos()->set_locationz(monster->_patrolPos._z);

                // 이동
                monster->_ownerRoom->ActorMove(monster, info);
            }
        }

    }

    // 있다 -> 공격 or 추적
    else {
        // 순찰 해제
        monster->_isPatrol = false;

        // 공격 사거리 안 = 공격
        if (dt <= monster->_info.stat().attackdistance())
            monster->ChangeState(StateAttack::Instance());
        
        // 공격 사거리 밖 = 추적
        else
            monster->ChangeState(StateChase::Instance());
    }
}

void StateIdle::Exit(shared_ptr<Monster> monster)
{
}

/*-------------------------------------------------------------------
    Chase
-------------------------------------------------------------------*/
StateChase* StateChase::Instance()
{
    static StateChase instance;
    return &instance;
}

MONSTER_STATE StateChase::GetType()
{
    return MONSTER_STATE::CHASE;
}

void StateChase::Enter(shared_ptr<Monster> monster)
{
    // 이동
    PROTOCOL::ObjectInfo nextPos;
    if (monster->GetNextPos(nextPos) == false)
        return;

    monster->_ownerRoom->ActorMove(monster, nextPos);
}

void StateChase::Execute(shared_ptr<Monster> monster)
{
    // 타겟이 유효한가
    shared_ptr<Player> target = monster->_target.lock();
    if (target) {
        const float& monsterX = monster->_info.pos().locationx();
        const float& monsterY = monster->_info.pos().locationy();
        const float& targetX = target->_info.pos().locationx();
        const float& targetY = target->_info.pos().locationy();

        // 타겟이 공격 사거리 안 - 공격 상태 변환
        if (monster->_ownerRoom->DistanceToTargetSimple(monsterX - targetX, monsterY - targetY) <= pow(monster->_info.stat().attackdistance(), 2)) 
            monster->ChangeState(StateAttack::Instance());

        // 타겟이 공격 사거리 밖 - Chase 유지
        else {
            // 추적 범위 밖
            if (pow(monster->_info.stat().returndistance(), 2) <= 
                monster->_ownerRoom->DistanceToTargetSimple(monster->_basePos._x - monsterX, monster->_basePos._y - monsterY)) {
                
                monster->ChangeState(StateReturn::Instance());
            }

            // 추적 범위 안
            else {
                PROTOCOL::ObjectInfo nextPos;
                if (monster->GetNextPos(nextPos) == false)
                    return;

                monster->_ownerRoom->ActorMove(monster, nextPos);
            }
        }
    }

    // 타겟이 유효하지 않음
    else {
        // 타겟 해제 후 원래 자리로 복귀
        monster->_target.reset();
        monster->ChangeState(StateReturn::Instance());
    }

}

void StateChase::Exit(shared_ptr<Monster> monster)
{

}

/*-------------------------------------------------------------------
    Attack
-------------------------------------------------------------------*/
StateAttack* StateAttack::Instance()
{
    static StateAttack instance;
    return &instance;
}

MONSTER_STATE StateAttack::GetType()
{
    return MONSTER_STATE::ATTACK;
}

void StateAttack::Enter(shared_ptr<Monster> monster)
{
    // 공격 쿨타임 확인
    uint64 nowTime = GetTickCount64();
    if (monster->_nextAttackTime > nowTime)
        return;

    monster->Skill(nowTime);
}

void StateAttack::Execute(shared_ptr<Monster> monster)
{
    // 타겟이 유효
    shared_ptr<Player> target = monster->_target.lock();
    if (target != nullptr && monster->_ownerRoom != nullptr) {
        float dx = target->_info.pos().locationx() - monster->_info.pos().locationx();
        float dy = target->_info.pos().locationy() - monster->_info.pos().locationy();

        // 공격 사거리 안
        if (pow(monster->_info.stat().attackdistance(), 2) >= monster->_ownerRoom->DistanceToTargetSimple(dx, dy)) {
            // 공격 쿨타임 확인
            uint64 nowTime = GetTickCount64();
            if (monster->_nextAttackTime > nowTime)
                return;

            // 공격
            monster->Skill(nowTime);
        }

        // 공격 사거리 밖
        else {
            // 추격 상태로 변경
            monster->ChangeState(StateChase::Instance());
        }
    }
    
    // 타겟이 유효하지 않음
    else {
        // 타겟 날리고, 리턴상태로 변경
        monster->_target.reset();
        monster->ChangeState(StateReturn::Instance());
    }
}

void StateAttack::Exit(shared_ptr<Monster> monster)
{
}

/*-------------------------------------------------------------------
    Return
-------------------------------------------------------------------*/
StateReturn* StateReturn::Instance()
{
    static StateReturn Instance;
    return &Instance;
}

MONSTER_STATE StateReturn::GetType()
{
    return MONSTER_STATE::RETURN;
}

void StateReturn::Enter(shared_ptr<Monster> monster)
{
    // TODO : 돌아갈 때 피격을 받지 않게
    // 풀피
    monster->_info.mutable_stat()->set_hp(monster->_info.mutable_stat()->maxhp());

    PROTOCOL::ObjectInfo nextPos;
    monster->GetNextPos(nextPos);
    monster->_ownerRoom->ActorMove(monster, nextPos);
}

void StateReturn::Execute(shared_ptr<Monster> monster)
{
    // 베이스 위치
    if (monster->_basePos._x == monster->_info.pos().locationx() &&
        monster->_basePos._y == monster->_info.pos().locationy()) {
        
        // 유휴 상태로 변경
        monster->ChangeState(StateIdle::Instance());
    }

    // 베이스 위치가 아님
    else {
        // 베이스로 이동
        PROTOCOL::ObjectInfo nextPos;
        monster->GetNextPos(nextPos);
        monster->_ownerRoom->ActorMove(monster, nextPos);
    }
}

void StateReturn::Exit(shared_ptr<Monster> monster)
{
}

/*-------------------------------------------------------------------
    Dead
-------------------------------------------------------------------*/
StateDead* StateDead::Instance()
{
    static StateDead instance;
    return &instance;
}

MONSTER_STATE StateDead::GetType()
{
    return MONSTER_STATE::DEAD;
}

void StateDead::Enter(shared_ptr<Monster> monster)
{
    cout << "Monster Dead Enter" << endl;
}

void StateDead::Execute(shared_ptr<Monster> monster)
{
}

void StateDead::Exit(shared_ptr<Monster> monster)
{
}


