#include "pch.h"
#include "ClientSession.h"

#include"ClientPacketHandler.h"
#include"SessionManager.h"

#include"GameObject.h"
#include"Room.h"

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

void ClientSession::OnConnected()
{
	SessionManager::Instance()->AddSession(static_pointer_cast<ClientSession>(shared_from_this()));
	_playerState = PROTOCOL::PlayerServerState::SERVER_STATE_CONNECTED;
}

void ClientSession::OnDisconnected()
{
	// �÷��̾ ������
	if (_player != nullptr && _player->_ownerRoom != nullptr) {
		cout << "Player-" << _player->_info.objectid() << ", Disconnect" << endl;

		_player->_ownerRoom->DoAsync(&Room::LeaveRoom, _player->_info.objectid());

		// ������Ʈ �Ŵ������� �÷��̾� ����
		ObjectManager::Instance()->Remove(_player->_info.objectid());

		// �÷��̾� ���� ����
		_player.reset();
	}
	else if (_player != nullptr && _player->_ownerRoom == nullptr) {
		cout << "Player-" << _player->_info.objectid() << ", Disconnect(Not In Room)" << endl;

		ObjectManager::Instance()->Remove(_player->_info.objectid());

		_player.reset();
	}

	// ���� �Ŵ������� �� ���� ����
	SessionManager::Instance()->RemoveSession(static_pointer_cast<ClientSession>(shared_from_this()));
}

void ClientSession::OnSend(int len)
{
}

void ClientSession::OnRecvPacket(char* buffer, int len)
{
	ClientPacketHandler::HandlePacket(static_pointer_cast<ClientSession>(shared_from_this()), buffer, len);

}

void ClientSession::SendPacket(shared_ptr<SendBuffer> sendBuffer)
{
	// �� �ɰ�
	lock_guard<mutex> lock(_packetBufferLock);

	// �迭�� Ǫ��
	_packetBuffer.push_back(sendBuffer);
}

void ClientSession::SendPacketFlush()
{
	lock_guard<mutex> lock(_packetBufferLock);

	if (_packetBuffer.empty())
		return;

	vector<shared_ptr<SendBuffer>, StlAllocator<shared_ptr<SendBuffer>>> tmp;
	tmp.swap(_packetBuffer);

	Send(move(tmp));
}

/*
	
*/
void ClientSession::HandleLogin(PROTOCOL::C_Login fromPkt)
{
	PROTOCOL::S_Login toPkt;

	// ������ ���
	//JwtHandler::VerifyToken(fromPkt.tokenstring());

	// ������
	auto dbConn = DBManager::Instance()->_gameDbManager->Pop();
	SP::SelectPlayerByAccountDbId selectPlayer(*dbConn);

	PlayerDB playerDB;
	selectPlayer.In_AccountDbId(stoi(fromPkt.tokenstring()));
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
			
			info.mutable_pos()->set_locationx(playerDB.LocationX);
			info.mutable_pos()->set_locationy(playerDB.LocationY);
			info.mutable_pos()->set_locationz(playerDB.LocationZ);
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

	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	SendPacket(sendBuffer);

	DBManager::Instance()->_gameDbManager->Push(dbConn);
}

void ClientSession::HandleCreatePlayer(string name)
{

	// TODO : �κ񿡼��� ���� �����ϰ�
	// ��Ŷ
	PROTOCOL::S_CreatePlayer toPkt;

	// DB Ŀ�ؼ�
	DBConnection* dbConn = GDBConnectionPool->Pop();

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
		newCharacterInfo.mutable_pos()->set_locationx(playerDB.LocationX);
		newCharacterInfo.mutable_pos()->set_locationy(playerDB.LocationY);
		newCharacterInfo.mutable_pos()->set_locationz(playerDB.LocationZ);

		_myCharacterList.push_back(newCharacterInfo);

		// ��Ŷ�� �� ĳ���� ���� �߰�
		toPkt.mutable_object()->CopyFrom(newCharacterInfo);
	}

	// ����
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	SendPacket(sendBuffer);

	// DB �ݳ�
	GDBConnectionPool->Push(dbConn);
}


void ClientSession::HandleEnterRoom(PROTOCOL::C_Enter_Room fromPkt) {
	//
	auto dbConn = DBManager::Instance()->_gameDbManager->Pop();

	// ���� �������� �ƴ� ó������
	if (_player == nullptr) {
		cout << "ClientSession::HandleEnterRoom()" << endl;

		PROTOCOL::S_ItemList itemListPkt;
		PROTOCOL::S_QuestList questListPtk;

		// �� ĳ�� ��� �߿��� Ŭ�� ������ ĳ���� ã�� -> �ش� ĳ�� �� ����
		for (PROTOCOL::ObjectInfo& info : _myCharacterList) {
			if (info.name().compare(fromPkt.object().name()) != 0)
				continue;
			
			// ������ƮǮ���� �ش� ��ü(�÷��̾�) ����, ���� ����
			_player = static_pointer_cast<Player>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::PLAYER));
			_player->_ownerSession = static_pointer_cast<ClientSession>(shared_from_this());
			_player->_questManager->_ownerPlayer = _player; // �ϴ� ���� (�����ڿ��� �������ϱ� player�� null) 

			// �÷��̾� ���� ����
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
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(itemListPkt);
			SendPacket(sendBuffer);
		}
		{ // ����Ʈ ��Ŷ
			auto sendBuffer = ClientPacketHandler::MakeSendBuffer(questListPtk);
			SendPacket(sendBuffer);
		}
	}

	// ���� ������
	else {
		cout << "ClientSession::HandleEnterRoom() ReEnterRoom" << endl;

		// �׳� hp 200���� ��Ȱ
		_player->_info.mutable_stat()->set_hp(200);
	}

	RoomManager::Instance()->Find(1)->DoAsync(&Room::EnterRoom, static_pointer_cast<GameObject>(_player));

	DBManager::Instance()->_gameDbManager->Push(dbConn);
}