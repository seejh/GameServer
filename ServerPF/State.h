#pragma once


enum MONSTER_STATE : uint8 {
	IDLE, CHASE, ATTACK, RETURN, DEAD,
};

class Monster;
class State
{
public:
	virtual ~State() {};
	virtual MONSTER_STATE GetType() = 0;
	virtual void Enter(shared_ptr<Monster> monster) = 0;
	virtual void Execute(shared_ptr<Monster> monster) = 0;
	virtual void Exit(shared_ptr<Monster> monster) = 0;
};

class StateIdle : public State {
public:
	static StateIdle* Instance();
	virtual MONSTER_STATE GetType() override;
	virtual void Enter(shared_ptr<Monster> monster) override;
	virtual void Execute(shared_ptr<Monster> monster) override;
	virtual void Exit(shared_ptr<Monster> monster) override;
private:
	StateIdle(){}
	StateIdle(const StateIdle&);
	StateIdle& operator=(const StateIdle&);
};

class StateChase : public State {
public:
	static StateChase* Instance();
	virtual MONSTER_STATE GetType() override;
	virtual void Enter(shared_ptr<Monster> monster) override;
	virtual void Execute(shared_ptr<Monster> monster) override;
	virtual void Exit(shared_ptr<Monster> monster) override;
private:
	StateChase() {}
	StateChase(const StateChase&);
	StateChase& operator=(const StateChase&);
};

class StateAttack : public State {
public:
	static StateAttack* Instance();
	virtual MONSTER_STATE GetType() override;
	virtual void Enter(shared_ptr<Monster> monster) override;
	virtual void Execute(shared_ptr<Monster> monster) override;
	virtual void Exit(shared_ptr<Monster> monster) override;
private:
	StateAttack(){}
	StateAttack(const StateAttack&);
	StateAttack& operator=(const StateAttack&);
};

class StateReturn : public State {
public:
	static StateReturn* Instance();
	virtual MONSTER_STATE GetType() override;
	virtual void Enter(shared_ptr<Monster> monster) override;
	virtual void Execute(shared_ptr<Monster> monster) override;
	virtual void Exit(shared_ptr<Monster> monster) override;
private:
	StateReturn(){}
	StateReturn(const StateReturn&);
	StateReturn& operator=(const StateReturn&);
};

class StateDead : public State {
public:
	static StateDead* Instance();
	virtual MONSTER_STATE GetType() override;
	virtual void Enter(shared_ptr<Monster> monster) override;
	virtual void Execute(shared_ptr<Monster> monster) override;
	virtual void Exit(shared_ptr<Monster> monster) override;
private:
	StateDead(){}
	StateDead(const StateDead&);
	StateDead& operator=(const StateDead&);
};