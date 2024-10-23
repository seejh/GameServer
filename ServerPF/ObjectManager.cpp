#include "pch.h"
#include "ObjectManager.h"

#include"GameObject.h"

ObjectManager::ObjectManager() {

}

shared_ptr<GameObject> ObjectManager::Add(PROTOCOL::GameObjectType objectType)
{
	// 락 걸고
	lock_guard<mutex> lock(_m);
	shared_ptr<GameObject> object;
	
	// ID 생성 발급
	int id = GeneratedId(objectType);

	// 각 종류별로 오브젝트 생성 후 오브젝트 매니저에 추가
	switch (objectType) {
	case PROTOCOL::GameObjectType::PLAYER: 
	{
		shared_ptr<Player> player = make_shared<Player>();

		object = static_pointer_cast<GameObject>(player);
		_objects[id] = object;
	} break;


	case PROTOCOL::GameObjectType::MONSTER: 
	{
		shared_ptr<Monster> monster = make_shared<Monster>();
		
		// 오브젝트 매니저에 추가, 따로 보관 관리는 하지 않는다.
		// 자체적으로 콜백이 계속 호출되는데, 몬스터가 사망할 경우...
		object = static_pointer_cast<GameObject>(monster);
		_objects[id] = object;
	} break;

	case PROTOCOL::GameObjectType::NPC:
	{
		shared_ptr<Npc> npc = make_shared<Npc>();

		object = static_pointer_cast<GameObject>(npc);
		_objects[id] = object;
	}

	case PROTOCOL::GameObjectType::Object:
	{

	}

	default:
		break;
	}
	
	// ID, 타입 설정
	object->_info.set_objectid(id);
	object->_info.set_objecttype(objectType);

	return object;
}

bool ObjectManager::Remove(int objectId)
{
	PROTOCOL::GameObjectType objectType = GetObjectTypeById(objectId);

	lock_guard<mutex> lock(_m);

	if (objectType == PROTOCOL::GameObjectType::PLAYER)
		return _objects.erase(objectId);

	return false;
}

int ObjectManager::GeneratedId(PROTOCOL::GameObjectType type)
{
	const int counter = _counter.fetch_add(1);
	return ((int)type << 24 | (counter));
}

PROTOCOL::GameObjectType ObjectManager::GetObjectTypeById(int id)
{
	int type =  (id >> 24) & 0x7F;

	switch (type) {
	case PROTOCOL::GameObjectType::PLAYER:
		return PROTOCOL::GameObjectType::PLAYER;
	case PROTOCOL::GameObjectType::MONSTER:
		return PROTOCOL::GameObjectType::MONSTER;
	case PROTOCOL::GameObjectType::NPC:
		return PROTOCOL::GameObjectType::NPC;
	default:
		return PROTOCOL::GameObjectType::NONE;
	}
}

shared_ptr<GameObject> ObjectManager::Find(int objectId)
{
	PROTOCOL::GameObjectType type = GetObjectTypeById(objectId);

	lock_guard<mutex> lock(_m);

	// 플레이어
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		auto it = _objects.find(objectId);
		if (it != _objects.end())
			return it->second;
	}

	// 몬스터
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		auto it = _objects.find(objectId);
		if (it != _objects.end())
			return it->second;
	}

	// NPC
	else if (type == PROTOCOL::GameObjectType::NPC) {
		auto it = _objects.find(objectId);
		if (it != _objects.end())
			return it->second;
	}

	return nullptr;
}
