#pragma once




#include"JobQueue.h"
#include"Zone.h"
#include"DBDataModel.h"

class GameObject;
class ClientSession;
class Monster;
class Player;
class SendBuffer;
class Room : public JobQueue
{
public:
	Room() {}
	~Room();

	void Init(int minX, int maxX, int minY, int maxY);

	// Adjacent 연산량에 대해서
	Zone* GetZone(float targetX, float targetY);
	void GetAdjacentZone(float targetX, float targetY, vector<Zone*>& zones);
	float DistanceToTargetSimple(float dx, float dy);
	float DistanceToTarget(float dx, float dy);

	void Update();

	void EnterRoom(shared_ptr<GameObject> object);
	void LeaveRoom(int objectId);

	// 룸 브로드캐스트, 패킷 바로 보낼지 기다렸다 보낼지, 일단 이렇게
	void Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer);
	void BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer);

	void ActorMove(shared_ptr<GameObject> gameObject, PROTOCOL::ObjectInfo info);
	void ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_Skill fromPkt);
	void ActorUseItem(shared_ptr<GameObject> gameObject, PROTOCOL::C_UseItem fromPkt);
	void PlayerAddItem(shared_ptr<Player> player, PROTOCOL::ItemInfo info);
	void PlayerChat(shared_ptr<Player> player, PROTOCOL::C_Chat fromPkt);

	float GetPlayerAround(shared_ptr<Monster> monster);
private:
	
public:
	uint64 _lastUpdateTime = 0;

	int _width = 1000;
	int _minX;
	int _minY;
	int _countX;
	int _countY;
	vector<vector<Zone*>> _zones;

	map<int32, shared_ptr<Player>> _players;
	map<int32, shared_ptr<Monster>> _monsters;

};

