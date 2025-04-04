#include "pch.h"
#include "GameSession.h"

#include"GameClientPacketHandler.h"
#include"GameSessionManager.h"

#include"GameObject.h"
#include"GameRoom.h"

#include"DBDataModel.h"
#include"DBConnectionPool.h"
#include"GenProcedures.h"
#include"DBSynchronizer.h"
#include"Item.h"
#include"Inventory.h"
#include"RoomManager.h"
#include"ObjectManager.h"
#include"RedisManager.h"
#include"JwtHandler.h"
#include"DBManager.h"
#include"GameDBManager.h"
#include"Quest.h"
#include"QuestManager.h"

void GameSession::OnConnected()
{
	//SessionManager::Instance()->AddSession(static_pointer_cast<ClientSession>(shared_from_this()));
	//_playerState = PROTOCOL::PlayerServerState::SERVER_STATE_CONNECTED;

	GameSessionManager::Instance()->DoAsync(&GameSessionManager::AddSession, static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnDisconnected()
{
	// ���� �ʿ�
	// ��������, �÷��̾ �� �ȿ� ����
	if (_player != nullptr && _player->_ownerRoom != nullptr) {
		cout << "[Session] Disconnect Player-" << _player->_info.objectid() << endl;

		_player->_ownerRoom->DoAsync(&GameRoom::LeaveRoom, _player->_info.objectid());

		// �� �κ��� ���� �־�� �ϴ°� Ȯ��
		
		
		// �÷��̾� ���� ����
		_player.reset();
	}
	// ���� ����, �÷��̾ ����
	else if (_player != nullptr && _player->_ownerRoom == nullptr) {
		cout << "[Session] Disconnect Player-" << _player->_info.objectid() << "(Not in Room)" << endl;

		ObjectManager::Instance()->Remove(_player->_info.objectid());

		_player.reset();
	}

	// ���� �Ŵ������� �� ���� ����
	// SessionManager::Instance()->RemoveSession(static_pointer_cast<ClientSession>(shared_from_this()));

	GameSessionManager::Instance()->DoAsync(&GameSessionManager::RemoveSession, static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnSend(int len)
{
}

void GameSession::OnRecvPacket(char* buffer, int len)
{
	shared_ptr<PacketSession> packetSession = static_pointer_cast<PacketSession>(shared_from_this());
	GameClientPacketHandler::HandlePacket(packetSession, buffer, len);
}

void GameSession::SendPacket(shared_ptr<SendBuffer> sendBuffer)
{
	// �� �ɰ�
	lock_guard<mutex> lock(_mutex2);

	// �迭�� Ǫ��
	_packetBuffer.push_back(sendBuffer);
}

void GameSession::SendPacketFlush()
{
	lock_guard<mutex> lock(_mutex2);

	if (_packetBuffer.empty())
		return;

	vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>> tmp;
	tmp.swap(_packetBuffer);

	Send(move(tmp));
}

/*
	
*/
void GameSession::HandleLogin(PROTOCOL::C_LOGIN fromPkt)
{
	PROTOCOL::S_LOGIN toPkt;

	//
	int32 accountDbId;
	if (JwtHandler::VerifyToken(fromPkt.tokenstring(), accountDbId) == false) 
		return;

	// 
	string token = "";
	RedisConnection* redisConn = DBManager::Instance()->_redisDbManager->Pop();
	if (redisConn->GetKey(to_string(accountDbId).c_str(), token) == false)
		return;
	if (token.compare(fromPkt.tokenstring()) != 0)
		return;

	// 
	auto dbConn = DBManager::Instance()->_gameDbManager->Pop();
	SP::SelectPlayerByAccountDbId selectPlayer(*dbConn);

	PlayerDB playerDB;
	selectPlayer.In_AccountDbId(accountDbId);
	selectPlayer.Out_PlayerDbId(playerDB.PlayerDbId);
	selectPlayer.Out_PlayerName(playerDB.PlayerName);
	selectPlayer.Out_AccountDbId(playerDB.AccountDbId);
	selectPlayer.Out_LocationX(playerDB.LocationX);
	selectPlayer.Out_LocationY(playerDB.LocationY);
	selectPlayer.Out_LocationZ(playerDB.LocationZ);
	selectPlayer.Out_Level(playerDB.Level);
	selectPlayer.Out_TotalExp(playerDB.Exp);
	selectPlayer.Out_MaxHp(playerDB.MaxHp);
	selectPlayer.Out_Hp(playerDB.Hp);
	selectPlayer.Out_Damage(playerDB.Damage);
	if (selectPlayer.Execute()) {
		while (selectPlayer.Fetch()) {
			PROTOCOL::ObjectInfo info;
			info.set_objecttype(PROTOCOL::GameObjectType::PLAYER);
			info.set_playerdbid(playerDB.PlayerDbId);

			wstring wstr(playerDB.PlayerName);
			int needSize = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
			string str(needSize, 0);
			WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], needSize, NULL, NULL);
			info.set_name(str);
			info.mutable_pos()->mutable_location()->CopyFrom(MakePFVector(playerDB.LocationX, playerDB.LocationY, playerDB.LocationZ));
			info.mutable_stat()->set_level(playerDB.Level);
			info.mutable_stat()->set_totalexp(DataManager::Instance()->_statTable[playerDB.Level].totalExp);
			info.mutable_stat()->set_exp(playerDB.Exp);
			info.mutable_stat()->set_maxhp(playerDB.MaxHp);
			info.mutable_stat()->set_hp(playerDB.Hp);
			info.mutable_stat()->set_damage(playerDB.Damage);

			//
			_myCharacterList.push_back(info);
			//
			toPkt.add_objectinfos()->CopyFrom(info);
		}
	}

	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	SendPacket(sendBuffer);

	DBManager::Instance()->_gameDbManager->Push(dbConn);
}

void GameSession::HandleCreatePlayer(string name)
{

	// TODO : �κ񿡼��� ���� �����ϰ�
	// ��Ŷ
	PROTOCOL::S_CREATE_PLAYER toPkt;

	// DB Ŀ�ؼ�
	DBConnection* dbConn = DBManager::Instance()->_gameDbManager->Pop();
	 
	// ĳ���� �̸� (STR -> WSTR) 
	int needSize = MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), NULL, 0);
	wstring wstr(needSize, 0);
	MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), &wstr[0], needSize);
	wstr.append(L"\0");
	
	// ���̵� �ߺ� Ȯ��
	int32 num;
	SP::SelectPlayerName selectPlayer(*dbConn);
	selectPlayer.In_PlayerName(wstr.c_str(), wstr.size());
	selectPlayer.Out_PlayerDbId(num);
	if (selectPlayer.Execute() == false)
		return;

	// �ߺ�
	if (selectPlayer.Fetch() == true)
		toPkt.mutable_object()->set_name("Name Exists");
	
	// �ߺ� X, ��밡��
	else {
		// 1���� ����
		StatData stat = DataManager::Instance()->_statTable[1];

		// 
		PlayerDB playerDB;
		wcscpy_s(playerDB.PlayerName, wstr.c_str());
		playerDB.AccountDbId = _accountDbId;
		playerDB.Level = stat.level;
		playerDB.Exp = 0;
		playerDB.MaxHp = stat.maxhp;
		playerDB.Hp = stat.maxhp;
		playerDB.Damage = stat.damage;
		playerDB.LocationX = 0.f;
		playerDB.LocationY = -1000.f;
		playerDB.LocationZ = 118.14999389648438f;
		
		// DB ����
		int64 outIdentity = -1;
		SP::InsertPlayerSelectIdentity insertPlayer(*dbConn);
		insertPlayer.In_PlayerName(playerDB.PlayerName);
		insertPlayer.In_AccountDbId(playerDB.AccountDbId);
		insertPlayer.In_Level(playerDB.Level);
		insertPlayer.In_TotalExp(playerDB.Exp);
		insertPlayer.In_MaxHp(playerDB.MaxHp);
		insertPlayer.In_Hp(playerDB.MaxHp);
		insertPlayer.In_Damage(playerDB.Damage);
		insertPlayer.In_LocationX(playerDB.LocationX);
		insertPlayer.In_LocationY(playerDB.LocationY);
		insertPlayer.In_LocationZ(playerDB.LocationZ);
		insertPlayer.Out_Identity(outIdentity);
		if (insertPlayer.Execute() == false) {
			cout << "ClientSession::CreatePlayer() - InsertPlayer Execute Error" << endl;
			return;
		}
		if (insertPlayer.Fetch() == false) {
			cout << "ClientSession::CreatePlayer() - InsertPlayer Fetch Error" << endl;
			return;
		}
		playerDB.PlayerDbId = outIdentity;


		// �⺻ ������ ����
		// �⺻ ������ - ��
		SP::InsertItem insertItem(*dbConn);
		insertItem.In_PlayerDbId(playerDB.PlayerDbId);
		insertItem.In_TemplateId(1);
		insertItem.In_Count(1);
		insertItem.In_Slot(0);
		insertItem.In_Equipped(false);
		if (insertItem.Execute() == false) {
			cout << "ClientSession::CreatePlayer() - InsertItem Error" << endl;
			return;
		}

		// �⺻ ������ - ����
		SP::InsertItem insertItem2(*dbConn);
		insertItem2.In_PlayerDbId(playerDB.PlayerDbId);
		insertItem2.In_TemplateId(200);
		insertItem2.In_Count(10);
		insertItem2.In_Slot(1);
		insertItem2.In_Equipped(false);
		if (insertItem2.Execute() == false) {
			cout << "ClientSession::CreatePlayer() - InsertItem2 Error" << endl;
			return;
		}

		// (����) �� ���� ���� ĳ���� ��� �߰�
		PROTOCOL::ObjectInfo newCharacterInfo;
		newCharacterInfo.set_name(name);
		newCharacterInfo.set_playerdbid(playerDB.PlayerDbId);
		newCharacterInfo.set_objecttype(PROTOCOL::GameObjectType::PLAYER);
		newCharacterInfo.mutable_stat()->set_level(playerDB.Level);
		newCharacterInfo.mutable_stat()->set_hp(playerDB.Hp);
		newCharacterInfo.mutable_stat()->set_maxhp(playerDB.MaxHp);
		newCharacterInfo.mutable_stat()->set_damage(playerDB.Damage);
		newCharacterInfo.mutable_stat()->set_totalexp(stat.totalExp);
		newCharacterInfo.mutable_stat()->set_exp(playerDB.Exp);
		newCharacterInfo.mutable_pos()->mutable_location()->CopyFrom(MakePFVector(playerDB.LocationX, playerDB.LocationY, playerDB.LocationZ));
		
		// (���� �޸�) ���� ĳ���� �߰�
		_myCharacterList.push_back(newCharacterInfo);

		// ���� ĳ���� ��Ŷ
		toPkt.mutable_object()->CopyFrom(newCharacterInfo);
	}

	// ����
	auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(toPkt);
	SendPacket(sendBuffer);

	// DB �ݳ�
	DBManager::Instance()->_gameDbManager->Push(dbConn);
}


void GameSession::HandleEnterRoom(PROTOCOL::C_ENTER_ROOM fromPkt) {
	//
	auto dbConn = DBManager::Instance()->_gameDbManager->Pop();

	// ���� �������� �ƴ� ó������
	if (_player == nullptr) {
		PROTOCOL::S_ITEMLIST itemListPkt;
		PROTOCOL::S_QUESTLIST questListPtk;

		// �� ĳ�� ��� �߿��� Ŭ�� ������ ĳ���� ã�� -> �ش� ĳ�� �� ����
		for (PROTOCOL::ObjectInfo& info : _myCharacterList) {
			if (info.name().compare(fromPkt.object().name()) != 0)
				continue;
			
			// �÷��̾� ��ü ����, ���� ����
			_player = static_pointer_cast<Player>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::PLAYER));
			_player->_ownerSession = static_pointer_cast<GameSession>(shared_from_this());
			_player->_questManager->_ownerPlayer = _player; // �ϴ� ���� (�����ڿ��� �������ϱ� player�� null) 

			// ������ �÷��̾� ��ü�� ���� �����Ϸ��� ĳ���� ���� ����
			info.set_objectid(_player->_info.objectid());
			info.set_objecttype(_player->_info.objecttype());
			_player->_info.CopyFrom(info);
			_player->_playerDbId = _player->_info.playerdbid();

			// DB������ ��ȸ -> ������ ����(����) -> �κ��丮�� �߰�(����)
			ItemDB itemDB;
			SP::SelectItemByPlayerDbId selectItem(*dbConn);
			selectItem.In_PlayerDbId(_player->_playerDbId);
			selectItem.Out_ItemDbId(itemDB.ItemDbId);
			selectItem.Out_TemplateId(itemDB.TemplateId);
			selectItem.Out_Count(itemDB.Count);
			selectItem.Out_Slot(itemDB.Slot);
			selectItem.Out_Equipped(itemDB.Equipped);
			selectItem.Out_PlayerDbId(itemDB.PlayerDbId);
			if (selectItem.Execute()) {
				while (selectItem.Fetch()) {

					shared_ptr<Item> item = Item::MakeItem(itemDB);
					if (item) {
						_player->_inven.Add(item);
						itemListPkt.add_items()->CopyFrom(item->_itemInfo);
					}
				}
			}

			// DB����Ʈ ��ȸ -> ����Ʈ ����(����) -> ����Ʈ�Ŵ����� �߰�(����)
			QuestDB questDB;
			SP::SelectQuestByPlayerDbId selectQuest(*dbConn);
			selectQuest.In_PlayerDbId(_player->_playerDbId);
			selectQuest.Out_QuestDbId(questDB.QuestDbId);
			selectQuest.Out_TemplateId(questDB.TemplateId);
			selectQuest.Out_PlayerDbId(questDB.PlayerDbId);
			selectQuest.Out_Completed(questDB.Completed);
			selectQuest.Out_Progress(questDB.Progress);
			if (selectQuest.Execute()) {
				while (selectQuest.Fetch()) {

					shared_ptr<Quest> quest = Quest::MakeQuest(questDB);
					if (quest) {
						_player->_questManager->Add(quest);
						questListPtk.add_quests()->CopyFrom(quest->_questInfo);
					}
				}
			}
		}

		{ // ������ ��Ŷ
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(itemListPkt);
			SendPacket(sendBuffer);
		}
		{ // ����Ʈ ��Ŷ
			auto sendBuffer = GameClientPacketHandler::MakeSendBuffer(questListPtk);
			SendPacket(sendBuffer);
		}
	}

	// �������� �� �߰��Ǵ� ���� - �ϴ� ���ͷ����� ���
	if (fromPkt.isrespawn() == true) {
		_player->_info.mutable_stat()->set_hp(200);
	}

	RoomManager::Instance()->Find(1)->DoAsync(&GameRoom::EnterRoom, static_pointer_cast<GameObject>(_player));

	DBManager::Instance()->_gameDbManager->Push(dbConn);
}




// �� �α���
void GameSession::TestBotLogin()
{
	// ù ����
	if (_player == nullptr) {
		// �÷��̾� ��ü ����
		_player = static_pointer_cast<Player>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::PLAYER));

		// �÷��̾�� �� ��ü(����) ����, ��ȣ����
		_player->_ownerSession = static_pointer_cast<GameSession>(shared_from_this());

		// ������(�⺻Į)
		ItemDB itemDB;
		itemDB.ItemDbId = 0;
		itemDB.TemplateId = 1;
		itemDB.PlayerDbId = 1;
		itemDB.Count = 1;
		itemDB.Slot = 0;
		itemDB.Equipped = true;
		shared_ptr<Item> item = Item::MakeItem(itemDB);
		if (item)
			_player->_inven.Add(item);

		// �� �÷��� ����
		_player->_isBot = true;
	}
	else {
		cout << "ClientSession::TestBotLogin() _player != nullptr" << endl;
	}
	
	// ù����, ������ ����
	
	// �÷��̾� ���� - 5���� ����
	StatData stat = DataManager::Instance()->_statTable[3];
	PlayerDB playerDB;
	playerDB.Level = stat.level;
	playerDB.Exp = 0;
	playerDB.MaxHp = stat.maxhp;
	playerDB.Hp = stat.maxhp;
	playerDB.Damage = stat.damage;
	playerDB.LocationX = 0.f;
	playerDB.LocationY = -1000.f;
	playerDB.LocationZ = 118.275002f;
	
	// ����
	_player->_info.mutable_stat()->set_level(stat.level);
	_player->_info.mutable_stat()->set_exp(0);
	_player->_info.mutable_stat()->set_totalexp(stat.totalExp);
	_player->_info.mutable_stat()->set_maxhp(stat.maxhp);
	_player->_info.mutable_stat()->set_hp(stat.maxhp);
	_player->_info.mutable_stat()->set_damage(stat.damage);

	// ��ġ
	_player->_info.mutable_pos()->mutable_location()->CopyFrom(MakePFVector(0.f, -1000.f, 118.149993f));
	
	// ������
	RoomManager::Instance()->Find(1)->DoAsync(&GameRoom::EnterRoom, static_pointer_cast<GameObject>(_player));
}
