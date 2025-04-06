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
    // ���� �ȿ� Ÿ��(�÷��̾�)�� �ִ°�
    if (monster->_ownerRoom == nullptr)
        return;

    // Ÿ�ٰ��� �Ÿ�
    float dtToTarget = monster->_ownerRoom->GetPlayerAround(monster);

    // ���� -> ���� �Ǵ� ���
    if (dtToTarget < 0) {

        // ���� ���� ���̶��
        if (monster->_isPatrol) {
            
            // ���� ��ġ �����ߴٸ�
            if (monster->_info.pos().location().x() == monster->_patrolPos.x() &&
                monster->_info.pos().location().y() == monster->_patrolPos.y()) {

                cout << "Arrived At PatrolPos, Patrol Off" << endl;

                // ���� ����, ���� ��Ÿ�� ������Ʈ
                monster->_isPatrol = false;
                monster->_nextPatrolTime = monster->_nowUpdateTime + 7000;
            }

            // ���� ���̸� ���� ���� �������� ���� ���� ����
            else {
                // ��� ��û
                cout << "Not Arrived At PatrolPos" << endl;
                PROTOCOL::PFVector destPos;
                if (monster->_ownerRoom->FindPath(monster, monster->_patrolPos, destPos) == true)
                    monster->_ownerRoom->ActorMove(monster, destPos);
            }
        }

        // ���� ���� ���� �ƴ϶��
        else {
            // ���� �� Ȯ��
            if (monster->_nextPatrolTime < monster->_nowUpdateTime) {
                
                // ���� ��ġ ��ȸ
                if (monster->_ownerRoom->FindRandomPos(monster) == true) {
                    // ���� �÷���
                    monster->_isPatrol = true;
                    cout << "Find PatrolPos OK" << endl;

                    PROTOCOL::PFVector destPos;
                    if (monster->_ownerRoom->FindPath(monster, monster->_patrolPos, destPos) == true)
                        monster->_ownerRoom->ActorMove(monster, destPos);
                }
            }
        }

    }

    // �ִ� -> ���� �Ǵ� ����
    else {
        // ���� ����
        monster->_isPatrol = false;

        // ���� ��Ÿ� �� = ����
        if (dtToTarget <= monster->_info.stat().attackdistance())
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
    // �̵� ��� ��û
    shared_ptr<Player> target = monster->_target.lock();

    // ��� ��û
    PROTOCOL::PFVector destPos;
    if (monster->_ownerRoom->FindPath(monster, target->_info.pos().location(), destPos) == true)
        monster->_ownerRoom->ActorMove(monster, destPos);
}

void StateChase::Execute(shared_ptr<Monster> monster)
{
    // Ÿ���� ��ȿ�Ѱ�
    shared_ptr<Player> target = monster->_target.lock();
    if (target) {

        // Ÿ���� ���� ��Ÿ� �� - ���� ���� ��ȯ (����, sqrt ���� ���)
        if (pow(monster->_info.stat().attackdistance(), 2) >= monster->_ownerRoom->GetDistanceXYSimple(
            target->_info.pos().location().x() - monster->_info.pos().location().x(), 
            target->_info.pos().location().y() - monster->_info.pos().location().y())) {

            monster->ChangeState(StateAttack::Instance());
        }

        // Ÿ���� ���� ��Ÿ� �� - Chase ����
        else {
            // ���� ���� ��
            if (pow(monster->_info.stat().returndistance(), 2) <= monster->_ownerRoom->GetDistanceXYSimple(
                monster->_basePos.x() - monster->_info.pos().location().x(), 
                monster->_basePos.y() - monster->_info.pos().location().y())) {
                
                monster->ChangeState(StateReturn::Instance());
            }

            // ���� ���� ��
            else {
                // �̵� ��� ��û
                shared_ptr<Player> target = monster->_target.lock();

                //
                PROTOCOL::PFVector destPos;
                if (monster->_ownerRoom->FindPath(monster, target->_info.pos().location(), destPos) == true)
                    monster->_ownerRoom->ActorMove(monster, destPos);
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
    if (monster->_nextAttackTime > monster->_nowUpdateTime)
        return;

    // ���� ó��
    monster->Skill();
}

void StateAttack::Execute(shared_ptr<Monster> monster)
{
    // Ÿ���� ��ȿ
    shared_ptr<Player> target = monster->_target.lock();
    if (target != nullptr && monster->_ownerRoom != nullptr) {
        float dx = target->_info.pos().location().x() - monster->_info.pos().location().x();
        float dy = target->_info.pos().location().y() - monster->_info.pos().location().y();

        // ���� ��Ÿ� �� (����, sqrt���� ���)
        if (pow(monster->_info.stat().attackdistance(), 2) >= monster->_ownerRoom->GetDistanceXYSimple(
            target->_info.pos().location().x() - monster->_info.pos().location().x(),
            target->_info.pos().location().y() - monster->_info.pos().location().y())) {

            // ���� ��Ÿ�� Ȯ��
            if (monster->_nextAttackTime > monster->_nowUpdateTime)
                return;

            // ���� ó��
            monster->Skill();
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
    // �̵� ��� ��û
    // TODO : ���ư� �� �ǰ��� ���� �ʰ�, Ǯ��, ������ ������Ʈ�� �˷�����
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
    // �⺻ ��ġ ����
    if (monster->_basePos.x() == monster->_info.pos().location().x() &&
        monster->_basePos.y() == monster->_info.pos().location().y()) {

        // ���� ���·� ����
        monster->ChangeState(StateIdle::Instance());
    }

    // �⺻ ��ġ�� �ƴ�
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


