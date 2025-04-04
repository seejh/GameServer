#pragma once


/*-----------------------------
	Yaw; // z
	Pitch; //y
	Roll; // x
----------------------------*/

#include"JobQueue.h"
#include"Zone.h"
#include"DBDataModel.h"


class GameMap;
class AIServerSession;
class GameObject;
class GameSession;
class Monster;
class Player;
class SendBuffer;
class GameRoom : public JobQueue
{
public:
	GameRoom();

	void Init(); // TODO : GameRoom Config
	// void Update();

	// 
	shared_ptr<Zone>& GetZone(float targetX, float targetY);
	void GetAdjacentZone(float targetX, float targetY, vector<shared_ptr<Zone>>& zones);
	
	float GetDistance(float dX, float dY, float dZ);
	float GetDistanceSimple(float dX, float dY, float dZ);
	float GetDistanceXY(float dX, float dY);
	float GetDistanceXYSimple(float dX, float dY);
	
	// 
	void EnterRoom(shared_ptr<GameObject> object);
	void LeaveRoom(int objectId);
	void HandleUseItem(shared_ptr<Player> player, PROTOCOL::C_USE_ITEM fromPkt);
	void HandleEquipItem(shared_ptr<Player> player, PROTOCOL::C_EQUIP_ITEM fromPkt);
	void HandleAddQuest(shared_ptr<Player> player, PROTOCOL::C_ADD_QUEST fromPkt);
	void HandleRemoveQuest(shared_ptr<Player> player, PROTOCOL::C_REMOVE_QUEST fromPkt);
	void HandleCompleteQuest(shared_ptr<Player> player, PROTOCOL::C_COMPLETE_QUEST fromPkt);
	void HandleUpdateQuest(shared_ptr<Player> player, PROTOCOL::C_UPDATE_QUEST fromPkt);

	// DB
	// 
	void DB_Response_UseItem(shared_ptr<Player> player, ItemDB itemDB);
	void DB_Response_AddItem(shared_ptr<Player> player, ItemDB itemDB);

	// 
	void DB_Response_RemoveQuest(shared_ptr<Player> player, QuestDB questDB);
	void DB_Response_AddQuest(shared_ptr<Player> player, QuestDB questDB);
	void DB_Response_CompleteQuest(shared_ptr<Player> player, QuestDB questDB, vector<ItemDB> itemDBs);

	// 룸 브로드캐스트, 패킷 바로 보낼지 기다렸다 보낼지, 일단 이렇게
	void Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer);
	void BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer);

	//
	void ActorMove(shared_ptr<GameObject> gameObject, PROTOCOL::PFVector destPos);
	void ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_SKILL fromPkt);
	void PlayerChat(shared_ptr<Player> player, PROTOCOL::C_CHAT fromPkt);

	float GetPlayerAround(shared_ptr<Monster> monster);

	// TEST
	PROTOCOL::PFVector CalculatePathFind(shared_ptr<Monster>& monster, vector<PROTOCOL::PFVector>& paths);
	//void AI_Request_PathFind(shared_ptr<Monster> monster, PROTOCOL::PFVector targetPos);
	//void AI_Response_PathFind(shared_ptr<AIServerSession> aiServerSession, PROTOCOL::AI_S_PATHFIND fromPkt);
	//void AI_Request_PathFindRandom(shared_ptr<Monster> monster);
	//void AI_Response_PathFindRandom(shared_ptr<AIServerSession> aiServerSession, PROTOCOL::AI_S_PATHFIND_RANDOM fromPkt);
	//void AddAISesison(shared_ptr<AIServerSession> aiServerSession);

	bool FindPath(shared_ptr<Monster> monster, PROTOCOL::PFVector ep, PROTOCOL::PFVector& destPos);
	bool FindRandomPos(shared_ptr<Monster> monster);

private:
	inline void SubV(PROTOCOL::PFVector* dest, PROTOCOL::PFVector* loc1, PROTOCOL::PFVector* loc2) {
		dest->set_x(loc1->x() - loc2->x());
		dest->set_y(loc1->y() - loc2->y());
		dest->set_z(loc1->z() - loc2->z());
	}
	inline void SumV(PROTOCOL::PFVector* dest, PROTOCOL::PFVector* loc1, PROTOCOL::PFVector* loc2) {
		dest->set_x(loc1->x() + loc2->x());
		dest->set_y(loc1->y() + loc2->y());
		dest->set_z(loc1->z() + loc2->z());
	}
	inline void Normalize(PROTOCOL::PFVector* loc) {
		double dt =sqrt(pow(loc->x(), 2) + pow(loc->y(), 2) + pow(loc->z(), 2));

		loc->set_x(loc->x() / dt);
		loc->set_y(loc->y() / dt);
		loc->set_z(loc->z() / dt);
	}
public:
	uint64 _lastUpdateTime = 0;

	// TEST
	// shared_ptr<AIServerSession> _aiSession = nullptr;

	shared_ptr<GameMap> _gameMap;
};

