#include "pch.h"
#include "Room.h"


#include"GameObject.h"
#include"ClientSession.h"
#include"ClientPacketHandler.h"
#include"ObjectManager.h"
#include"Item.h"


Room::~Room()
{
	for (vector<Zone*> v : _zones)
		for (Zone* z : v)
			delete(z);
}

// 일단 정수에서 나누어 떨어지게 맵의 크기를 조정할 것임
void Room::Init(int minX, int maxX, int minY, int maxY)
{
	_minX = minX;
	_minY = minY;

	int sizeX = abs(maxX - minX);
	int sizeY = abs(maxY - minY);

	_countX = sizeX / _width;
	_countY = sizeY / _width;

	//
	_zones.resize(_countY);
	for (int y = 0; y < _countY; y++) {
		
		_zones[y].resize(_countX);
		for (int x = 0; x < _countX; x++) {

			int baseX = minX + (_width * x);
			int baseY = minY + (_width * y);
			
			// cout << "[" << y << "," << x << "] = [" << baseX << "," << baseY << "]" << endl;
			_zones[y][x] = new Zone(baseX, baseY, _width);
		}
	}

	// 나중에 수정
	{// 몬스터1
		// 몬스터 생성, 설정 초기화(0번 몬스터 설정)
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// 베이스 임의 위치
		monster->_basePos._x = 2000;
		monster->_basePos._y = -3000;
		monster->_basePos._z = 145.f;

		// 위치
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		// 몬스터 게임룸 진입
		EnterRoom(monster);
	}
	{// 몬스터2
		// 몬스터 생성, 설정 초기화(0번 몬스터 설정)
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// 베이스 임의 위치
		monster->_basePos._x = 4000;
		monster->_basePos._y = 0;
		monster->_basePos._z = 145.f;

		// 위치
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		// 몬스터 게임룸 진입
		EnterRoom(monster);
	}
	{// 몬스터3
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos._x = 6000;
		monster->_basePos._y = -3000;
		monster->_basePos._z = 145.f;

		// 위치
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		EnterRoom(monster);
	}
	{// 몬스터4
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos._x = 8000;
		monster->_basePos._y = 0;
		monster->_basePos._z = 145.f;

		// 위치
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		EnterRoom(monster);
	}
}

Zone* Room::GetZone(float targetX, float targetY)
{
	int indexX = abs(_minX - targetX) / _width;
	int indexY = abs(_minY - targetY) / _width;

	return _zones[indexY][indexX];
}

void Room::GetAdjacentZone(float targetX, float targetY, vector<Zone*>& zones)
{
	int indexX = abs(_minX - targetX) / _width;
	int indexY = abs(_minY - targetY) / _width;

	for (int x = indexX - 1; x <= indexX + 1; x++) {
		if (x < 0 || x >= _countX)
			continue;

		for (int y = indexY - 1; y <= indexY + 1; y++) {
			if (y < 0 || y >= _countY)
				continue;

			zones.push_back(_zones[y][x]);
		}
	}
}

float Room::DistanceToTargetSimple(float dx, float dy)
{
	return (pow(abs(dx), 2) + pow(abs(dy), 2));
}

float Room::DistanceToTarget(float dx, float dy)
{
	return sqrt(DistanceToTargetSimple(dx, dy));
}

void Room::Update()
{
	Execute();
}

void Room::EnterRoom(shared_ptr<GameObject> object)
{
	

	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(object->_info.objectid());
	PROTOCOL::S_Spawn spawnPkt;

	// 플레이어
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// 
		if (_players.count(object->_info.objectid()) == 0) { 
			cout << "PLAYER-" << object->_info.objectid() << ", EnterRoom" << endl;

			shared_ptr<Player> player = static_pointer_cast<Player>(object);

			// 룸에 플레이어 추가
			_players[player->_info.objectid()] = player;

			// 플레이어에서의 룸 참조 추가
			player->_ownerRoom = static_pointer_cast<Room>(shared_from_this());
			
			// 아이템 기반으로 해당 캐릭터의 스탯을 계산
			player->CalculateAddStat();

			// 플레이어 맵, 존 추가
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->_players.insert(player);
			
			// 당사자에게만, S_Enter_Room
			PROTOCOL::S_Enter_Room enterRoomPkt;
			enterRoomPkt.set_success(true);
			enterRoomPkt.mutable_object()->CopyFrom(player->_info);
			
			// S_Enter_Room 패킷 전송
			shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(enterRoomPkt);
			player->_ownerSession->SendPacket(sendBuffer);
			
			// 플레이어 비전 활성화
			player->_vision._ownerPlayer = player;
			player->_vision.Update();

			// 브로드캐스트(플레이어면 여기서 브로드캐스트하고 종료)
			/*spawnPkt.add_object()->CopyFrom(player->_info);
			sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
			BroadcastWithoutMe(player, sendBuffer);*/

			return;
		}
	}
	// 몬스터
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		 
		if (_monsters.count(object->_info.objectid()) == 0) {
			cout << "MONSTER-" << object->_info.objectid() << ", EnterRoom" << endl;

			shared_ptr<Monster> monster = static_pointer_cast<Monster>(object);

			// 몬스터 게임 룸에 추가 및 몬스터에서의 룸 참조
			_monsters[monster->_info.objectid()] = monster;
			monster->_ownerRoom = static_pointer_cast<Room>(shared_from_this());

			// 몬스터 맵, 존에 추가
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->_monsters.insert(monster);

			// 몬스터 업데이트 시동
			monster->Update();

			// 스폰 브로드 캐스트
			spawnPkt.add_object()->CopyFrom(monster->_info);
		}
	}
	// 투사체, 기타등등...
	else {
		return;
	}
	
	// 브로드캐스트 - 스폰 패킷(플레이어가 아니라면)
	//shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
	//Broadcast(object->_info.pos().locationx(), object->_info.pos().locationy(), sendBuffer);
}

void Room::LeaveRoom(int objectId)
{
	cout << "Actor-" << objectId << ", LeaveRoom" << endl;

	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(objectId);
	PROTOCOL::S_DeSpawn despawnPkt;

	// 플레이어
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		auto it = _players.find(objectId);
		if (it != _players.end()) {
			// 
			shared_ptr<Player> player = it->second;
			
			// 맵(존)에서 플레이어 제거
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->Remove(player);

			// 룸에서 플레이어 제거
			_players.erase(objectId);

			// 플레이어 스탯 저장(DB)
			player->OnLeaveGame();
			
			// 예약 작업 취소
			shared_ptr<Job> reservedJob = player->_reservedJob.lock();
			reservedJob->_cancel = true;

			// 플레이어 룸 이탈 패킷
			PROTOCOL::S_Leave_Room leaveRoomPkt;
			leaveRoomPkt.mutable_object()->set_objectid(player->_info.objectid());
			leaveRoomPkt.mutable_object()->set_objecttype(player->_info.objecttype());
			
			// 본인에게만 전달
			shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(leaveRoomPkt);
			player->_ownerSession->SendPacket(sendBuffer);
			
			// 브로드 캐스팅 (플레이어면 여기서 브로드캐스팅하고 종료)
			/*despawnPkt.add_objectids(objectId);
			sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
			BroadcastWithoutMe(player, sendBuffer);
			*/

			// 플레이어 비전 비활성화
			player->_vision._prevObjects.clear();
			player->_vision._ownerPlayer.reset();
			
			// 플레이어 룸 참조 제거 (메모리 참조 때문에 브로드캐스트 후)
			player->_ownerRoom.reset();

			// 플레이어 세션 참조 제거 (메모리 참조 때문에 브로드캐스트 후)
			// player->_ownerSession.reset();

			return;
		}
	}
	// 몬스터
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		auto it = _monsters.find(objectId);
		if (it != _monsters.end()) {
			// 
			shared_ptr<Monster> monster = it->second;

			// 맵(존)에서 몬스터 제거
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->Remove(monster);

			// 룸에서 몬스터 제거
			_monsters.erase(objectId);

			// 예약 작업 취소
			shared_ptr<Job> reservedJob = monster->_reservedJob.lock();
			reservedJob->_cancel = true;

			// 몬스터의 방 참조 제거
			monster->_ownerRoom.reset();
		}
	}
	// 투사체, 기타등등
	else {
		return;
	}

	// 브로드캐스팅
	//shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
	//Broadcast(object->_info.pos().locationx(), object->_info.pos().locationy(), sendBuffer);
}

void Room::Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer)
{
	// 브로드캐스트할 존 추출
	vector<Zone*> zones;
	GetAdjacentZone(locX, locY, zones);

	// 추출한 존의 모든 플레이어에게 브로드캐스팅
	for (Zone* z : zones) 
		for (shared_ptr<Player> p : z->_players) 
			p->_ownerSession->SendPacket(sendBuffer);

}

void Room::BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer)
{

	// 브로드캐스트할 존 추출
	vector<Zone*> zones;
	GetAdjacentZone(player->_info.pos().locationx(), player->_info.pos().locationy(), zones);

	// 추출한 존에서 본인 제외 모든 플레이어에게 브로드캐스팅
	for (Zone* z : zones) {
		for (shared_ptr<Player> p : z->_players) {
			if (player->_info.objectid() == p->_info.objectid())
				continue;

			p->_ownerSession->SendPacket(sendBuffer);
		}
	}
}

void Room::ActorMove(shared_ptr<GameObject> gameObject, PROTOCOL::ObjectInfo info)
{
	// 서버에서 위치 변경

	// 현위치
	float nowX = gameObject->_info.pos().locationx();
	float nowY = gameObject->_info.pos().locationy();
	Zone* nowZone = GetZone(nowX, nowY);

	// 이동할 위치
	float afterX = info.pos().locationx();
	float afterY = info.pos().locationy();
	Zone* afterZone = GetZone(afterX, afterY);

	// 존이 변경되면 위치 옮김
	if (nowZone->_zoneId != afterZone->_zoneId) {
		afterZone->Add(gameObject);
		nowZone->Remove(gameObject);
	}

	// 보고 받은 위치로 서버에서 위치 갱신
	gameObject->_info.mutable_pos()->set_locationx(info.pos().locationx());
	gameObject->_info.mutable_pos()->set_locationy(info.pos().locationy());
	gameObject->_info.mutable_pos()->set_locationz(info.pos().locationz());

	gameObject->_info.mutable_pos()->set_rotationpitch(info.pos().rotationpitch());
	gameObject->_info.mutable_pos()->set_rotationyaw(info.pos().rotationyaw());
	gameObject->_info.mutable_pos()->set_rotationroll(info.pos().rotationroll());
	
	gameObject->_info.mutable_pos()->set_velocityx(info.pos().velocityx());
	gameObject->_info.mutable_pos()->set_velocityy(info.pos().velocityy());
	gameObject->_info.mutable_pos()->set_velocityz(info.pos().velocityz());

	// 이동 패킷
	PROTOCOL::S_Move toPkt;
	toPkt.mutable_object()->CopyFrom(gameObject->_info);

	// 브로드 캐스팅
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(afterX, afterY, sendBuffer);
}

void Room::ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_Skill fromPkt)
{
	// 쿨타임 등 체크 OK - 일단 패스
	
	// 공격자 위치를 기반으로 공격 패킷 브로드캐스팅
	shared_ptr<GameObject> attacker = ObjectManager::Instance()->Find(fromPkt.attacker());
	if (attacker == nullptr)
		return;

	// 공격 패킷
	PROTOCOL::S_Skill toPkt;
	toPkt.set_attacker(fromPkt.attacker());
	toPkt.set_skillid(fromPkt.skillid());
	for (int victim : fromPkt.victims())
		toPkt.add_victims(victim);
	
	// 브로드캐스팅
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(attacker->_info.pos().locationx(), attacker->_info.pos().locationy(), sendBuffer);


	// 스킬 판정
	// 스킬 정보 확인
	auto it = DataManager::Instance()->_skillTable.find(fromPkt.skillid());
	if (it != DataManager::Instance()->_skillTable.end()) {
		// 스킬 정보
		SkillData skillData = it->second;
		
		// 스킬 종류에 따라 처리
		switch (skillData.skillType) {
		// 평타
		case PROTOCOL::SkillType::SKILL_AUTO: 
		{
			// 희생자들
			for (int i = 0; i < fromPkt.victims_size(); i++) {
				// 
				shared_ptr<GameObject> victim = ObjectManager::Instance()->Find(fromPkt.victims(i));
				if (victim == nullptr) 
					continue;

				float dx = gameObject->_info.pos().locationx() - victim->_info.pos().locationx();
				float dy = gameObject->_info.pos().locationy() - victim->_info.pos().locationy();

				// TEST - 사거리 안에 드는지, 일단 사거리 30 임의 지정
				float squaredDT = DistanceToTargetSimple(dx, dy);
				if (squaredDT <= pow(100, 2)) {
					
					// 총공격력 + 스킬 대미지
					// 플레이어 = 아이템 대미지 + 스텟 대미지 + 스킬 대미지
					// 몬스터 = 스텟 대미지 + 스킬 대미지
					victim->OnDamaged(attacker, attacker->_totalDamage + skillData.damage);
				}
			}
		} 
		break;

		// 그 외
		default:
			break;
		}
	}
}

void Room::ActorUseItem(shared_ptr<GameObject> gameObject, PROTOCOL::C_UseItem fromPkt)
{
	if(gameObject != nullptr)
		static_pointer_cast<Player>(gameObject)->UseItem(fromPkt);
}

void Room::PlayerAddItem(shared_ptr<Player> player, PROTOCOL::ItemInfo info)
{
	// 갱신
	auto it = player->_inven._items.find(info.itemdbid());
	if (it != player->_inven._items.end()) {
		player->_inven._items[info.itemdbid()]->_itemInfo.CopyFrom(info);
	}

	// 생성
	else {
		//
		ItemDB itemDB;
		itemDB.ItemDbId = info.itemdbid();
		itemDB.TemplateId = info.templateid();
		itemDB.Count = info.count();
		itemDB.Slot = info.slot();
		itemDB.Equipped = info.equipped();
		itemDB.PlayerDbId = info.playerdbid();

		// 아이템 생성
		shared_ptr<Item> item = Item::MakeItem(itemDB);
		if (item == nullptr)
			return;

		// 플레이어 인벤토리에 아이템 적재
		player->_inven.Add(item);
	}

	// 패킷 생성
	PROTOCOL::S_AddItem toPkt;
	toPkt.add_items()->CopyFrom(info);

	// 전송
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	player->_ownerSession->SendPacket(sendBuffer);
}

void Room::PlayerChat(shared_ptr<Player> player, PROTOCOL::C_Chat fromPkt)
{
	// 패킷 생성
	PROTOCOL::S_Chat toPkt;
	toPkt.mutable_object()->set_name(player->_info.name());
	toPkt.set_text(fromPkt.text());

	// 브로드캐스트
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(player->_info.pos().locationx(), player->_info.pos().locationy(), sendBuffer);
}

float Room::GetPlayerAround(shared_ptr<Monster> monster)
{
	// 몬스터의 위치
	const float& monsterX = monster->_info.pos().locationx();
	const float& monsterY = monster->_info.pos().locationy();

	// 몬스터 탐색 범위 - (간략=거리제곱)
	float squaredNoticeDT = monster->_info.stat().noticedistance();
	squaredNoticeDT = pow(squaredNoticeDT, 2);
	
	// 위치에 해당하는 주변 존
	vector<Zone*> zones;
	GetAdjacentZone(monsterX, monsterY, zones);

	for (Zone* z : zones) {
		// 존에서 플레이어 색출, 
		for (shared_ptr<Player> p : z->_players) {
			
			// 몬스터와 플레이어 간 거리(간략)
			float dt = DistanceToTargetSimple(monsterX - p->_info.pos().locationx(), monsterY - p->_info.pos().locationy());

			// 몬스터 탐색 범위(간략)과 몬스터플레이어간거리(간략) 비교
			if (dt <= squaredNoticeDT) {
				// 몬스터 타겟 설정
				monster->_target = p;

				// 실제값 반환
				return sqrt(dt);
			}
		}
	}
	
	// 플레이어 없으면 음수값 리턴
	return -1.f;
}

