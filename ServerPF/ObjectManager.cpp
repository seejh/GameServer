#include "pch.h"
#include "ObjectManager.h"

#include"GameObject.h"

ObjectManager::ObjectManager() {

}

shared_ptr<GameObject> ObjectManager::Add(PROTOCOL::GameObjectType objectType)
{
	// �� �ɰ�
	lock_guard<mutex> lock(_m);
	shared_ptr<GameObject> object;
	
	// ID ���� �߱�
	int id = GeneratedId(objectType);

	// �� �������� ������Ʈ ���� �� ������Ʈ �Ŵ����� �߰�
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
		
		// ������Ʈ �Ŵ����� �߰�, ���� ���� ������ ���� �ʴ´�.
		// ��ü������ �ݹ��� ��� ȣ��Ǵµ�, ���Ͱ� ����� ���...
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
	
	// ID, Ÿ�� ����
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

	// �÷��̾�
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		auto it = _objects.find(objectId);
		if (it != _objects.end())
			return it->second;
	}

	// ����
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
