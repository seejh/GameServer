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
    // ���� �ȿ� Ÿ��(�÷��̾�)�� �ִ°�
    // ���⼭ ownerroom�� nullptr�� ��� - ���� �װ��� ���� ��� �Ǳ��� ����
    // �ϴ�...
    if (monster->_ownerRoom == nullptr)
        return;

    float dt = monster->_ownerRoom->GetPlayerAround(monster);

    // ���� - ���� or ���
    if (dt < 0) {
        
        // ���� �� - ���� ��ġ�� ���������� ����
        if (monster->_isPatrol) {
            // ���� ��ġ ���� O
            if (monster->_info.pos().locationx() == monster->_patrolPos._x &&
                monster->_info.pos().locationy() == monster->_patrolPos._y) {
               
                // ���� ����, ���� ���� �ð� -> ���ľߵ�
                monster->_isPatrol = false;
                monster->_nextPatrolTime = GetTickCount64() + 7000;
            }

            // ���� ��ġ ���� X
            else {
                
                // ���� ��ġ�� ����ؼ� �̵�
                PROTOCOL::ObjectInfo info;
                monster->GetNextPos(info);
                monster->_ownerRoom->ActorMove(monster, info);
            }
        }

        // ���� �ƴ� - ���� ����
        else {
            uint64 nowTime = GetTickCount64();

            // ���� ���� �Ǿ��°�
            if (monster->_nextPatrolTime < nowTime) {
                
                // ���� ��ġ, ���� ����    ����
                monster->_patrolPos = monster->GetRandomPatrolPos();
                
                monster->_isPatrol = true;

                //
                PROTOCOL::ObjectInfo info;
                info.mutable_pos()->set_locationx(monster->_patrolPos._x);
                info.mutable_pos()->set_locationy(monster->_patrolPos._y);
                info.mutable_pos()->set_locationz(monster->_patrolPos._z);

                // �̵�
                monster->_ownerRoom->ActorMove(monster, info);
            }
        }

    }

    // �ִ� -> ���� or ����
    else {
        // ���� ����
        monster->_isPatrol = false;

        // ���� ��Ÿ� �� = ����
        if (dt <= monster->_info.stat().attackdistance())
            monster->ChangeState(StateAttack::Instance());
        
        // ���� ��Ÿ� �� = ����
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
    // �̵�
    PROTOCOL::ObjectInfo nextPos;
    if (monster->GetNextPos(nextPos) == false)
        return;

    monster->_ownerRoom->ActorMove(monster, nextPos);
}

void StateChase::Execute(shared_ptr<Monster> monster)
{
    // Ÿ���� ��ȿ�Ѱ�
    shared_ptr<Player> target = monster->_target.lock();
    if (target) {
        const float& monsterX = monster->_info.pos().locationx();
        const float& monsterY = monster->_info.pos().locationy();
        const float& targetX = target->_info.pos().locationx();
        const float& targetY = target->_info.pos().locationy();

        // Ÿ���� ���� ��Ÿ� �� - ���� ���� ��ȯ
        if (monster->_ownerRoom->DistanceToTargetSimple(monsterX - targetX, monsterY - targetY) <= pow(monster->_info.stat().attackdistance(), 2)) 
            monster->ChangeState(StateAttack::Instance());

        // Ÿ���� ���� ��Ÿ� �� - Chase ����
        else {
            // ���� ���� ��
            if (pow(monster->_info.stat().returndistance(), 2) <= 
                monster->_ownerRoom->DistanceToTargetSimple(monster->_basePos._x - monsterX, monster->_basePos._y - monsterY)) {
                
                monster->ChangeState(StateReturn::Instance());
            }

            // ���� ���� ��
            else {
                PROTOCOL::ObjectInfo nextPos;
                if (monster->GetNextPos(nextPos) == false)
                    return;

                monster->_ownerRoom->ActorMove(monster, nextPos);
            }
        }
    }

    // Ÿ���� ��ȿ���� ����
    else {
        // Ÿ�� ���� �� ���� �ڸ��� ����
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
    // ���� ��Ÿ�� Ȯ��
    uint64 nowTime = GetTickCount64();
    if (monster->_nextAttackTime > nowTime)
        return;

    monster->Skill(nowTime);
}

void StateAttack::Execute(shared_ptr<Monster> monster)
{
    // Ÿ���� ��ȿ
    shared_ptr<Player> target = monster->_target.lock();
    if (target != nullptr && monster->_ownerRoom != nullptr) {
        float dx = target->_info.pos().locationx() - monster->_info.pos().locationx();
        float dy = target->_info.pos().locationy() - monster->_info.pos().locationy();

        // ���� ��Ÿ� ��
        if (pow(monster->_info.stat().attackdistance(), 2) >= monster->_ownerRoom->DistanceToTargetSimple(dx, dy)) {
            // ���� ��Ÿ�� Ȯ��
            uint64 nowTime = GetTickCount64();
            if (monster->_nextAttackTime > nowTime)
                return;

            // ����
            monster->Skill(nowTime);
        }

        // ���� ��Ÿ� ��
        else {
            // �߰� ���·� ����
            monster->ChangeState(StateChase::Instance());
        }
    }
    
    // Ÿ���� ��ȿ���� ����
    else {
        // Ÿ�� ������, ���ϻ��·� ����
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
    // TODO : ���ư� �� �ǰ��� ���� �ʰ�
    // Ǯ��
    monster->_info.mutable_stat()->set_hp(monster->_info.mutable_stat()->maxhp());

    PROTOCOL::ObjectInfo nextPos;
    monster->GetNextPos(nextPos);
    monster->_ownerRoom->ActorMove(monster, nextPos);
}

void StateReturn::Execute(shared_ptr<Monster> monster)
{
    // ���̽� ��ġ
    if (monster->_basePos._x == monster->_info.pos().locationx() &&
        monster->_basePos._y == monster->_info.pos().locationy()) {
        
        // ���� ���·� ����
        monster->ChangeState(StateIdle::Instance());
    }

    // ���̽� ��ġ�� �ƴ�
    else {
        // ���̽��� �̵�
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


