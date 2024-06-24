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

// �ϴ� �������� ������ �������� ���� ũ�⸦ ������ ����
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

	// ���߿� ����
	{// ����1
		// ���� ����, ���� �ʱ�ȭ(0�� ���� ����)
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// ���̽� ���� ��ġ
		monster->_basePos._x = 2000;
		monster->_basePos._y = -3000;
		monster->_basePos._z = 145.f;

		// ��ġ
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		// ���� ���ӷ� ����
		EnterRoom(monster);
	}
	{// ����2
		// ���� ����, ���� �ʱ�ȭ(0�� ���� ����)
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(0);

		// ���̽� ���� ��ġ
		monster->_basePos._x = 4000;
		monster->_basePos._y = 0;
		monster->_basePos._z = 145.f;

		// ��ġ
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		// ���� ���ӷ� ����
		EnterRoom(monster);
	}
	{// ����3
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos._x = 6000;
		monster->_basePos._y = -3000;
		monster->_basePos._z = 145.f;

		// ��ġ
		monster->_info.mutable_pos()->set_locationx(monster->_basePos._x);
		monster->_info.mutable_pos()->set_locationy(monster->_basePos._y);
		monster->_info.mutable_pos()->set_locationz(monster->_basePos._z);

		EnterRoom(monster);
	}
	{// ����4
		shared_ptr<Monster> monster = static_pointer_cast<Monster>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::MONSTER));
		monster->Init(1);

		monster->_basePos._x = 8000;
		monster->_basePos._y = 0;
		monster->_basePos._z = 145.f;

		// ��ġ
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

	// �÷��̾�
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		// 
		if (_players.count(object->_info.objectid()) == 0) { 
			cout << "PLAYER-" << object->_info.objectid() << ", EnterRoom" << endl;

			shared_ptr<Player> player = static_pointer_cast<Player>(object);

			// �뿡 �÷��̾� �߰�
			_players[player->_info.objectid()] = player;

			// �÷��̾���� �� ���� �߰�
			player->_ownerRoom = static_pointer_cast<Room>(shared_from_this());
			
			// ������ ������� �ش� ĳ������ ������ ���
			player->CalculateAddStat();

			// �÷��̾� ��, �� �߰�
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->_players.insert(player);
			
			// ����ڿ��Ը�, S_Enter_Room
			PROTOCOL::S_Enter_Room enterRoomPkt;
			enterRoomPkt.set_success(true);
			enterRoomPkt.mutable_object()->CopyFrom(player->_info);
			
			// S_Enter_Room ��Ŷ ����
			shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(enterRoomPkt);
			player->_ownerSession->SendPacket(sendBuffer);
			
			// �÷��̾� ���� Ȱ��ȭ
			player->_vision._ownerPlayer = player;
			player->_vision.Update();

			// ��ε�ĳ��Ʈ(�÷��̾�� ���⼭ ��ε�ĳ��Ʈ�ϰ� ����)
			/*spawnPkt.add_object()->CopyFrom(player->_info);
			sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
			BroadcastWithoutMe(player, sendBuffer);*/

			return;
		}
	}
	// ����
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		 
		if (_monsters.count(object->_info.objectid()) == 0) {
			cout << "MONSTER-" << object->_info.objectid() << ", EnterRoom" << endl;

			shared_ptr<Monster> monster = static_pointer_cast<Monster>(object);

			// ���� ���� �뿡 �߰� �� ���Ϳ����� �� ����
			_monsters[monster->_info.objectid()] = monster;
			monster->_ownerRoom = static_pointer_cast<Room>(shared_from_this());

			// ���� ��, ���� �߰�
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->_monsters.insert(monster);

			// ���� ������Ʈ �õ�
			monster->Update();

			// ���� ��ε� ĳ��Ʈ
			spawnPkt.add_object()->CopyFrom(monster->_info);
		}
	}
	// ����ü, ��Ÿ���...
	else {
		return;
	}
	
	// ��ε�ĳ��Ʈ - ���� ��Ŷ(�÷��̾ �ƴ϶��)
	//shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(spawnPkt);
	//Broadcast(object->_info.pos().locationx(), object->_info.pos().locationy(), sendBuffer);
}

void Room::LeaveRoom(int objectId)
{
	cout << "Actor-" << objectId << ", LeaveRoom" << endl;

	PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(objectId);
	PROTOCOL::S_DeSpawn despawnPkt;

	// �÷��̾�
	if (type == PROTOCOL::GameObjectType::PLAYER) {
		auto it = _players.find(objectId);
		if (it != _players.end()) {
			// 
			shared_ptr<Player> player = it->second;
			
			// ��(��)���� �÷��̾� ����
			Zone* zone = GetZone(player->_info.pos().locationx(), player->_info.pos().locationy());
			zone->Remove(player);

			// �뿡�� �÷��̾� ����
			_players.erase(objectId);

			// �÷��̾� ���� ����(DB)
			player->OnLeaveGame();
			
			// ���� �۾� ���
			shared_ptr<Job> reservedJob = player->_reservedJob.lock();
			reservedJob->_cancel = true;

			// �÷��̾� �� ��Ż ��Ŷ
			PROTOCOL::S_Leave_Room leaveRoomPkt;
			leaveRoomPkt.mutable_object()->set_objectid(player->_info.objectid());
			leaveRoomPkt.mutable_object()->set_objecttype(player->_info.objecttype());
			
			// ���ο��Ը� ����
			shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(leaveRoomPkt);
			player->_ownerSession->SendPacket(sendBuffer);
			
			// ��ε� ĳ���� (�÷��̾�� ���⼭ ��ε�ĳ�����ϰ� ����)
			/*despawnPkt.add_objectids(objectId);
			sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
			BroadcastWithoutMe(player, sendBuffer);
			*/

			// �÷��̾� ���� ��Ȱ��ȭ
			player->_vision._prevObjects.clear();
			player->_vision._ownerPlayer.reset();
			
			// �÷��̾� �� ���� ���� (�޸� ���� ������ ��ε�ĳ��Ʈ ��)
			player->_ownerRoom.reset();

			// �÷��̾� ���� ���� ���� (�޸� ���� ������ ��ε�ĳ��Ʈ ��)
			// player->_ownerSession.reset();

			return;
		}
	}
	// ����
	else if (type == PROTOCOL::GameObjectType::MONSTER) {
		auto it = _monsters.find(objectId);
		if (it != _monsters.end()) {
			// 
			shared_ptr<Monster> monster = it->second;

			// ��(��)���� ���� ����
			Zone* zone = GetZone(monster->_info.pos().locationx(), monster->_info.pos().locationy());
			zone->Remove(monster);

			// �뿡�� ���� ����
			_monsters.erase(objectId);

			// ���� �۾� ���
			shared_ptr<Job> reservedJob = monster->_reservedJob.lock();
			reservedJob->_cancel = true;

			// ������ �� ���� ����
			monster->_ownerRoom.reset();
		}
	}
	// ����ü, ��Ÿ���
	else {
		return;
	}

	// ��ε�ĳ����
	//shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(despawnPkt);
	//Broadcast(object->_info.pos().locationx(), object->_info.pos().locationy(), sendBuffer);
}

void Room::Broadcast(float locX, float locY, shared_ptr<SendBuffer> sendBuffer)
{
	// ��ε�ĳ��Ʈ�� �� ����
	vector<Zone*> zones;
	GetAdjacentZone(locX, locY, zones);

	// ������ ���� ��� �÷��̾�� ��ε�ĳ����
	for (Zone* z : zones) 
		for (shared_ptr<Player> p : z->_players) 
			p->_ownerSession->SendPacket(sendBuffer);

}

void Room::BroadcastWithoutMe(shared_ptr<Player> player, shared_ptr<SendBuffer> sendBuffer)
{

	// ��ε�ĳ��Ʈ�� �� ����
	vector<Zone*> zones;
	GetAdjacentZone(player->_info.pos().locationx(), player->_info.pos().locationy(), zones);

	// ������ ������ ���� ���� ��� �÷��̾�� ��ε�ĳ����
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
	// �������� ��ġ ����

	// ����ġ
	float nowX = gameObject->_info.pos().locationx();
	float nowY = gameObject->_info.pos().locationy();
	Zone* nowZone = GetZone(nowX, nowY);

	// �̵��� ��ġ
	float afterX = info.pos().locationx();
	float afterY = info.pos().locationy();
	Zone* afterZone = GetZone(afterX, afterY);

	// ���� ����Ǹ� ��ġ �ű�
	if (nowZone->_zoneId != afterZone->_zoneId) {
		afterZone->Add(gameObject);
		nowZone->Remove(gameObject);
	}

	// ���� ���� ��ġ�� �������� ��ġ ����
	gameObject->_info.mutable_pos()->set_locationx(info.pos().locationx());
	gameObject->_info.mutable_pos()->set_locationy(info.pos().locationy());
	gameObject->_info.mutable_pos()->set_locationz(info.pos().locationz());

	gameObject->_info.mutable_pos()->set_rotationpitch(info.pos().rotationpitch());
	gameObject->_info.mutable_pos()->set_rotationyaw(info.pos().rotationyaw());
	gameObject->_info.mutable_pos()->set_rotationroll(info.pos().rotationroll());
	
	gameObject->_info.mutable_pos()->set_velocityx(info.pos().velocityx());
	gameObject->_info.mutable_pos()->set_velocityy(info.pos().velocityy());
	gameObject->_info.mutable_pos()->set_velocityz(info.pos().velocityz());

	// �̵� ��Ŷ
	PROTOCOL::S_Move toPkt;
	toPkt.mutable_object()->CopyFrom(gameObject->_info);

	// ��ε� ĳ����
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(afterX, afterY, sendBuffer);
}

void Room::ActorSkill(shared_ptr<GameObject> gameObject, PROTOCOL::C_Skill fromPkt)
{
	// ��Ÿ�� �� üũ OK - �ϴ� �н�
	
	// ������ ��ġ�� ������� ���� ��Ŷ ��ε�ĳ����
	shared_ptr<GameObject> attacker = ObjectManager::Instance()->Find(fromPkt.attacker());
	if (attacker == nullptr)
		return;

	// ���� ��Ŷ
	PROTOCOL::S_Skill toPkt;
	toPkt.set_attacker(fromPkt.attacker());
	toPkt.set_skillid(fromPkt.skillid());
	for (int victim : fromPkt.victims())
		toPkt.add_victims(victim);
	
	// ��ε�ĳ����
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(attacker->_info.pos().locationx(), attacker->_info.pos().locationy(), sendBuffer);


	// ��ų ����
	// ��ų ���� Ȯ��
	auto it = DataManager::Instance()->_skillTable.find(fromPkt.skillid());
	if (it != DataManager::Instance()->_skillTable.end()) {
		// ��ų ����
		SkillData skillData = it->second;
		
		// ��ų ������ ���� ó��
		switch (skillData.skillType) {
		// ��Ÿ
		case PROTOCOL::SkillType::SKILL_AUTO: 
		{
			// ����ڵ�
			for (int i = 0; i < fromPkt.victims_size(); i++) {
				// 
				shared_ptr<GameObject> victim = ObjectManager::Instance()->Find(fromPkt.victims(i));
				if (victim == nullptr) 
					continue;

				float dx = gameObject->_info.pos().locationx() - victim->_info.pos().locationx();
				float dy = gameObject->_info.pos().locationy() - victim->_info.pos().locationy();

				// TEST - ��Ÿ� �ȿ� �����, �ϴ� ��Ÿ� 30 ���� ����
				float squaredDT = DistanceToTargetSimple(dx, dy);
				if (squaredDT <= pow(100, 2)) {
					
					// �Ѱ��ݷ� + ��ų �����
					// �÷��̾� = ������ ����� + ���� ����� + ��ų �����
					// ���� = ���� ����� + ��ų �����
					victim->OnDamaged(attacker, attacker->_totalDamage + skillData.damage);
				}
			}
		} 
		break;

		// �� ��
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
	// ����
	auto it = player->_inven._items.find(info.itemdbid());
	if (it != player->_inven._items.end()) {
		player->_inven._items[info.itemdbid()]->_itemInfo.CopyFrom(info);
	}

	// ����
	else {
		//
		ItemDB itemDB;
		itemDB.ItemDbId = info.itemdbid();
		itemDB.TemplateId = info.templateid();
		itemDB.Count = info.count();
		itemDB.Slot = info.slot();
		itemDB.Equipped = info.equipped();
		itemDB.PlayerDbId = info.playerdbid();

		// ������ ����
		shared_ptr<Item> item = Item::MakeItem(itemDB);
		if (item == nullptr)
			return;

		// �÷��̾� �κ��丮�� ������ ����
		player->_inven.Add(item);
	}

	// ��Ŷ ����
	PROTOCOL::S_AddItem toPkt;
	toPkt.add_items()->CopyFrom(info);

	// ����
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	player->_ownerSession->SendPacket(sendBuffer);
}

void Room::PlayerChat(shared_ptr<Player> player, PROTOCOL::C_Chat fromPkt)
{
	// ��Ŷ ����
	PROTOCOL::S_Chat toPkt;
	toPkt.mutable_object()->set_name(player->_info.name());
	toPkt.set_text(fromPkt.text());

	// ��ε�ĳ��Ʈ
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	Broadcast(player->_info.pos().locationx(), player->_info.pos().locationy(), sendBuffer);
}

float Room::GetPlayerAround(shared_ptr<Monster> monster)
{
	// ������ ��ġ
	const float& monsterX = monster->_info.pos().locationx();
	const float& monsterY = monster->_info.pos().locationy();

	// ���� Ž�� ���� - (����=�Ÿ�����)
	float squaredNoticeDT = monster->_info.stat().noticedistance();
	squaredNoticeDT = pow(squaredNoticeDT, 2);
	
	// ��ġ�� �ش��ϴ� �ֺ� ��
	vector<Zone*> zones;
	GetAdjacentZone(monsterX, monsterY, zones);

	for (Zone* z : zones) {
		// ������ �÷��̾� ����, 
		for (shared_ptr<Player> p : z->_players) {
			
			// ���Ϳ� �÷��̾� �� �Ÿ�(����)
			float dt = DistanceToTargetSimple(monsterX - p->_info.pos().locationx(), monsterY - p->_info.pos().locationy());

			// ���� Ž�� ����(����)�� �����÷��̾�Ÿ�(����) ��
			if (dt <= squaredNoticeDT) {
				// ���� Ÿ�� ����
				monster->_target = p;

				// ������ ��ȯ
				return sqrt(dt);
			}
		}
	}
	
	// �÷��̾� ������ ������ ����
	return -1.f;
}

