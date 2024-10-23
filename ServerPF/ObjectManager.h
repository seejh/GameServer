#pragma once

// [1:No Meaning][7:Type][24:ID]



/*
	ObjectManager
	게임 오브젝트의 생성과 소멸을 담당
	다만, 생성된 오브젝트의 관리(보관)는 플레이어 타입만 관리를 함

	지켜볼 필요가 있음
*/
class GameObject;
class Player;
class ObjectManager
{
public:
	static ObjectManager* Instance() {
		static ObjectManager instance;
		return &instance;
	}
	
	//
	shared_ptr<GameObject> Add(PROTOCOL::GameObjectType objectType);
	bool Remove(int objectId);

	//
	int GeneratedId(PROTOCOL::GameObjectType type);
	PROTOCOL::GameObjectType GetObjectTypeById(int id);
	shared_ptr<GameObject> Find(int objectId);
private:
	ObjectManager();
	ObjectManager(const ObjectManager&);
	ObjectManager& operator=(const ObjectManager&);
public:
	mutex _m;
	map<int, shared_ptr<GameObject>> _objects;

	atomic<int> _counter = 1;
};

