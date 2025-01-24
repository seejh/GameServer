#pragma once

// [1:No Meaning][7:Type][24:ID]



/*
	ObjectManager
	
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
	void Remove(int objectId);

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

