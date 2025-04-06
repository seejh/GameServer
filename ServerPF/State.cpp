#include "pch.h"
#include "State.h"

#include"GameObject.h"
#include"GameRoom.h"
#include"GameClientPacketHandler.h"

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
    if (monster->_ownerRoom == nullptr)
        return;

    // 타겟과의 거리
    float dtToTarget = monster->_ownerRoom->GetPlayerAround(monster);

    // 없다 -> 순찰 또는 대기
    if (dtToTarget < 0) {

        // 현재 순찰 중이라면
        if (monster->_isPatrol) {
            
            // 순찰 위치 도착했다면
            if (monster->_info.pos().location().x() == monster->_patrolPos.x() &&
                monster->_info.pos().location().y() == monster->_patrolPos.y()) {

                cout << "Arrived At PatrolPos, Patrol Off" << endl;

                // 순찰 해제, 순찰 쿨타임 업데이트
                monster->_isPatrol = false;
                monster->_nextPatrolTime = monster->_nowUpdateTime + 7000;
            }

            // 순찰 중이며 아직 순찰 목적지에 도착 못한 상태
            else {
                // 경로 요청
                cout << "Not Arrived At PatrolPos" << endl;
                PROTOCOL::PFVector destPos;
                if (monster->_ownerRoom->FindPath(monster, monster->_patrolPos, destPos) == true)
                    monster->_ownerRoom->ActorMove(monster, destPos);
            }
        }

        // 현재 순찰 중이 아니라면
        else {
            // 순찰 쿨 확인
            if (monster->_nextPatrolTime < monster->_nowUpdateTime) {
                
                // 순찰 위치 조회
                if (monster->_ownerRoom->FindRandomPos(monster) == true) {
                    // 순찰 플래그
                    monster->_isPatrol = true;
                    cout << "Find PatrolPos OK" << endl;

                    PROTOCOL::PFVector destPos;
                    if (monster->_ownerRoom->FindPath(monster, monster->_patrolPos, destPos) == true)
                        monster->_ownerRoom->ActorMove(monster, destPos);
                }
            }
        }

    }

    // 있다 -> 공격 또는 추적
    else {
        // 순찰 해제
        monster->_isPatrol = false;

        // 공격 사거리 안 = 공격
        if (dtToTarget <= monster->_info.stat().attackdistance())
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
    // 이동 경로 요청
    shared_ptr<Player> target = monster->_target.lock();

    // 경로 요청
    PROTOCOL::PFVector destPos;
    if (monster->_ownerRoom->FindPath(monster, target->_info.pos().location(), destPos) == true)
        monster->_ownerRoom->ActorMove(monster, destPos);
}

void StateChase::Execute(shared_ptr<Monster> monster)
{
    // 타겟이 유효한가
    shared_ptr<Player> target = monster->_target.lock();
    if (target) {

        // 타겟이 공격 사거리 안 - 공격 상태 변환 (간단, sqrt 없는 계산)
        if (pow(monster->_info.stat().attackdistance(), 2) >= monster->_ownerRoom->GetDistanceXYSimple(
            target->_info.pos().location().x() - monster->_info.pos().location().x(), 
            target->_info.pos().location().y() - monster->_info.pos().location().y())) {

            monster->ChangeState(StateAttack::Instance());
        }

        // 타겟이 공격 사거리 밖 - Chase 유지
        else {
            // 추적 범위 밖
            if (pow(monster->_info.stat().returndistance(), 2) <= monster->_ownerRoom->GetDistanceXYSimple(
                monster->_basePos.x() - monster->_info.pos().location().x(), 
                monster->_basePos.y() - monster->_info.pos().location().y())) {
                
                monster->ChangeState(StateReturn::Instance());
            }

            // 추적 범위 안
            else {
                // 이동 경로 요청
                shared_ptr<Player> target = monster->_target.lock();

                //
                PROTOCOL::PFVector destPos;
                if (monster->_ownerRoom->FindPath(monster, target->_info.pos().location(), destPos) == true)
                    monster->_ownerRoom->ActorMove(monster, destPos);
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
    if (monster->_nextAttackTime > monster->_nowUpdateTime)
        return;

    // 공격 처리
    monster->Skill();
}

void StateAttack::Execute(shared_ptr<Monster> monster)
{
    // 타겟이 유효
    shared_ptr<Player> target = monster->_target.lock();
    if (target != nullptr && monster->_ownerRoom != nullptr) {
        float dx = target->_info.pos().location().x() - monster->_info.pos().location().x();
        float dy = target->_info.pos().location().y() - monster->_info.pos().location().y();

        // 공격 사거리 안 (간단, sqrt없는 계산)
        if (pow(monster->_info.stat().attackdistance(), 2) >= monster->_ownerRoom->GetDistanceXYSimple(
            target->_info.pos().location().x() - monster->_info.pos().location().x(),
            target->_info.pos().location().y() - monster->_info.pos().location().y())) {

            // 공격 쿨타임 확인
            if (monster->_nextAttackTime > monster->_nowUpdateTime)
                return;

            // 공격 처리
            monster->Skill();
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
    // 이동 경로 요청
    // TODO : 돌아갈 때 피격을 받지 않게, 풀피, 문제는 업데이트를 알려야함
    monster->_info.mutable_stat()->set_hp(monster->_info.mutable_stat()->maxhp());

    //
    if (monster->_ownerRoom) {
        PROTOCOL::PFVector destPos;
        if (monster->_ownerRoom->FindPath(monster, monster->_basePos, destPos) == true) 
            monster->_ownerRoom->ActorMove(monster, destPos);
    }
}

void StateReturn::Execute(shared_ptr<Monster> monster)
{
    // 기본 위치 도착
    if (monster->_basePos.x() == monster->_info.pos().location().x() &&
        monster->_basePos.y() == monster->_info.pos().location().y()) {

        // 유휴 상태로 변경
        monster->ChangeState(StateIdle::Instance());
    }

    // 기본 위치가 아님
    else {
        //
        if (monster->_ownerRoom) {
            PROTOCOL::PFVector destPos;
            if (monster->_ownerRoom->FindPath(monster, monster->_basePos, destPos) == true)
                monster->_ownerRoom->ActorMove(monster, destPos);
        }
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


