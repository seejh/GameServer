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
	// 이 플레이어의 위치
	float x = _ownerPlayer->_info.pos().location().x();
	float y = _ownerPlayer->_info.pos().location().y();

	// 플레이어 위치 기반으로 주위 존들 추출
	vector<shared_ptr<Zone>> zones;
	_ownerPlayer->_ownerRoom->GetAdjacentZone(x, y, zones);

	// 존들에서, 오브젝트 별로 따로 추출
	for (shared_ptr<Zone>& z : zones) {
		// 플레이어
		for (shared_ptr<Player> player : z->_players) {
			// 추출한 것이 자기 자신이면 패스
			if (player->_info.objectid() == _ownerPlayer->_info.objectid())
				continue;

			// 새 시야각에 추가
			newObjects.insert(player);
		}
		// 몬스터
		for (shared_ptr<Monster> monster : z->_monsters) {
			// 새 시야각에 추가
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
	// 새 시야각 오브젝트 목록
	set<shared_ptr<GameObject>> newObjects;
	Gather(newObjects);
	
	// DeSpawn 처리 - 시야에 보이던 오브젝트가 사라짐
	PROTOCOL::S_DESPAWN despawnPkt;
	{
		// 과거에 있었던 것
		for (shared_ptr<GameObject> prevObj : _prevObjects) {
			// 현재 없음 -> 디스폰 패킷에 추가
			if (newObjects.count(prevObj) == 0) 
				despawnPkt.add_objectids(prevObj->_info.objectid());
		}

		// 디스폰 패킷 전송
		if (despawnPkt.objectids_size() != 0) {
			shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(despawnPkt);
			_ownerPlayer->_ownerSession->SendPacket(sendBuffer);
		}
	}
	
	// Spawn 처리 - 시야에 새로운 오브젝트가 추가
	PROTOCOL::S_SPAWN spawnPkt; 
	{
		// 현재 있는게
		for (shared_ptr<GameObject> newObj : newObjects) {
			
			// 과거에 없었음
			if (_prevObjects.count(newObj) == 0) {
				PROTOCOL::ObjectInfo* info = spawnPkt.add_object();
				info->CopyFrom(newObj->_info);
			}
		}

		// 스폰 패킷 전송
		if (spawnPkt.object_size() != 0) {
			shared_ptr<SendBuffer> sendBuffer = GameClientPacketHandler::MakeSendBuffer(spawnPkt);
			_ownerPlayer->_ownerSession->SendPacket(sendBuffer);
		}
	}

	// 비전 큐브 오브젝트 업데이트
	_prevObjects.swap(newObjects);

	// 원본
	// 0.1초당 이 객체(자기 자신)를 업데이트 하게 콜백 예약, TODO : 수치조정
	//_ownerPlayer->_reservedJob = _ownerPlayer->_ownerRoom->DoTimer(1000, [this]() { Update(); });
	
	// 수정
	_ownerPlayer->_reservedJobs[PLAYERJOBS::PLAYER_JOB_VISION] = _ownerPlayer->_ownerRoom->DoTimer(1000, [this]() {Update(); });
}

