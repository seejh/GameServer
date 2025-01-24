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
	// void Update();
	
	// 
	Zone* GetZone(float targetX, float targetY);
	void GetAdjacentZone(float targetX, float targetY, vector<Zone*>& zones);
	float DistanceToTargetSimple(float dx, float dy);
	float DistanceToTarget(float dx, float dy);

	// 
	void EnterRoom(shared_ptr<GameObject> object);
	void LeaveRoom(int objectId);
	void HandleUseItem(shared_ptr<Player> player, PROTOCOL::C_UseItem fromPkt);
	void HandleEquipItem(shared_ptr<Player> player, PROTOCOL::C_EquipItem fromPkt);
	void HandleAddQuest(shared_ptr<Player> player, PROTOCOL::C_AddQuest fromPkt);
	void HandleRemoveQuest(shared_ptr<Player> player, PROTOCOL::C_RemoveQuest fromPkt);
	void HandleCompleteQuest(shared_ptr<Player> player, PROTOCOL::C_CompleteQuest fromPkt);
	void HandleUpdateQuest(shared_ptr<Player> player, PROTOCOL::C_UpdateQuest fromPkt);

	// DB
	// 
	void DBCallback_UseItem(shared_ptr<Player> player, ItemDB itemDB);
	void DBCallback_AddItem(shared_ptr<Player> player, ItemDB itemDB);

	// 
	void DBCallback_RemoveQuest(shared_ptr<Player> player, QuestDB questDB);
	void DBCallback_AddQuest(shared_ptr<Player> player, QuestDB questDB);
	void DBCallback_CompleteQuest(shared_ptr<Player> player, QuestDB questDB, vector<ItemDB> itemDBs);

	// 룸 브로드캐스트, 패킷 바로 보낼지 기다렸다 보낼지, 일단 이렇게
	void Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer);
	void BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer);

	//
	void ActorMove(shared_ptr<GameObject> gameObject, PROTOCOL::ObjectInfo info);
	void ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_Skill fromPkt);
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
	map<int32, shared_ptr<Npc>> _npcs;
};

