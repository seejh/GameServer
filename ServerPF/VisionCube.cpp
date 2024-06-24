#include "pch.h"
#include "VisionCube.h"

#include"GameObject.h"
#include"Zone.h"
#include"Room.h"
#include"ClientPacketHandler.h"

VisionCube::VisionCube()
{
}

void VisionCube::Gather(set<shared_ptr<GameObject>>& newObjects)
{
	// �� �÷��̾��� ��ġ
	float x = _ownerPlayer->_info.pos().locationx();
	float y = _ownerPlayer->_info.pos().locationy();

	// �÷��̾� ��ġ ������� ���� ���� ����
	vector<Zone*> zones;
	_ownerPlayer->_ownerRoom->GetAdjacentZone(x, y, zones);

	// ���鿡��, ������Ʈ ���� ���� ����
	for (Zone* z : zones) {
		// �÷��̾�
		for (shared_ptr<Player> p : z->_players) {
			// ������ ���� �ڱ� �ڽ��̸� �н�
			if (p->_info.objectid() == _ownerPlayer->_info.objectid())
				continue;

			// �� �þ߰��� �߰�
			newObjects.insert(p);
		}
		// ����
		for (shared_ptr<Monster> m : z->_monsters) {
			// �� �þ߰��� �߰�
			newObjects.insert(m);
		}

		// ����ü, �н�
		// NONE
	}
}

void VisionCube::Update()
{
	// �� �þ߰� ������Ʈ ���
	set<shared_ptr<GameObject>> newObjects;
	Gather(newObjects);
	
	// DeSpawn ó�� - �þ߿� ���̴� ������Ʈ�� �����
	PROTOCOL::S_DeSpawn despawnPkt;
	{
		// ���ſ� �־��� ��
		for (shared_ptr<GameObject> prevObj : _prevObjects) {
			// ���� ���� -> ���� ��Ŷ�� �߰�
			if (newObjects.count(prevObj) == 0) 
				despawnPkt.add_objectids(prevObj->_info.objectid());
		}

		// ���� ��Ŷ ����
		if (despawnPkt.objectids_size() != 0) {
			shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
			_ownerPlayer->_ownerSession->SendPacket(sendBuffer);
		}
	}
	
	// Spawn ó�� - �þ߿� ���ο� ������Ʈ�� �߰�
	PROTOCOL::S_Spawn spawnPkt; 
	{
		// ���� �ִ°�
		for (shared_ptr<GameObject> newObj : newObjects) {
			
			// ���ſ� ������
			if (_prevObjects.count(newObj) == 0) {
				PROTOCOL::ObjectInfo* info = spawnPkt.add_object();
				info->CopyFrom(newObj->_info);
			}
		}

		// ���� ��Ŷ ����
		if (spawnPkt.object_size() != 0) {
			shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
			_ownerPlayer->_ownerSession->SendPacket(sendBuffer);
		}
	}

	// ���� ť�� ������Ʈ ������Ʈ
	_prevObjects.swap(newObjects);

	// 0.1�ʴ� �� ��ü(�ڱ� �ڽ�)�� ������Ʈ �ϰ� �ݹ� ����, TODO : ��ġ����
	_ownerPlayer->_reservedJob = _ownerPlayer->_ownerRoom->DoTimer(1000, [this]() { Update(); });
}

