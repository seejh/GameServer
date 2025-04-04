#include "pch.h"
#include "VisionCube.h"

#include"GameObject.h"
#include"Zone.h"
#include"GameRoom.h"
#include"GameClientPacketHandler.h"
#include"GameSession.h"

VisionCube::VisionCube()
{
}

void VisionCube::Gather(set<shared_ptr<GameObject>>& newObjects)
{
	// �� �÷��̾��� ��ġ
	float x = _ownerPlayer->_info.pos().location().x();
	float y = _ownerPlayer->_info.pos().location().y();

	// �÷��̾� ��ġ ������� ���� ���� ����
	vector<shared_ptr<Zone>> zones;
	_ownerPlayer->_ownerRoom->GetAdjacentZone(x, y, zones);

	// ���鿡��, ������Ʈ ���� ���� ����
	for (shared_ptr<Zone>& z : zones) {
		// �÷��̾�
		for (shared_ptr<Player> player : z->_players) {
			// ������ ���� �ڱ� �ڽ��̸� �н�
			if (player->_info.objectid() == _ownerPlayer->_info.objectid())
				continue;

			// �� �þ߰��� �߰�
			newObjects.insert(player);
		}
		// ����
		for (shared_ptr<Monster> monster : z->_monsters) {
			// �� �þ߰��� �߰�
			newObjects.insert(monster);
		}

		// NPC
		for (shared_ptr<Npc> npc : z->_npcs) {
			newObjects.insert(npc);
		}
		
		// NONE
	}
}

void VisionCube::Update()
{
	// �� �þ߰� ������Ʈ ���
	set<shared_ptr<GameObject>> newObjects;
	Gather(newObjects);
	
	// DeSpawn ó�� - �þ߿� ���̴� ������Ʈ�� �����
	PROTOCOL::S_DESPAWN despawnPkt;
	{
		// ���ſ� �־��� ��
		for (shared_ptr<GameObject> prevObj : _prevObjects) {
			// ���� ���� -> ���� ��Ŷ�� �߰�
			if (newObjects.count(prevObj) == 0) 
				despawnPkt.add_objectids(prevObj->_info.objectid());
		}

		// ���� ��Ŷ ����
		if (despawnPkt.objectids_size() != 0) {
			shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(despawnPkt);
			_ownerPlayer->_ownerSession->SendPacket(sendBuffer);
		}
	}
	
	// Spawn ó�� - �þ߿� ���ο� ������Ʈ�� �߰�
	PROTOCOL::S_SPAWN spawnPkt; 
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
			shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(spawnPkt);
			_ownerPlayer->_ownerSession->SendPacket(sendBuffer);
		}
	}

	// ���� ť�� ������Ʈ ������Ʈ
	_prevObjects.swap(newObjects);

	// ����
	// 0.1�ʴ� �� ��ü(�ڱ� �ڽ�)�� ������Ʈ �ϰ� �ݹ� ����, TODO : ��ġ����
	//_ownerPlayer->_reservedJob = _ownerPlayer->_ownerRoom->DoTimer(1000, [this]() { Update(); });
	
	// ����
	_ownerPlayer->_reservedJobs[PLAYERJOBS::PLAYER_JOB_VISION] = _ownerPlayer->_ownerRoom->DoTimer(1000, [this]() {Update(); });
}

