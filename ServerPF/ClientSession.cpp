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

void ClientSession::OnConnected()
{
	SessionManager::Instance()->AddSession(static_pointer_cast<ClientSession>(shared_from_this()));
	_playerState = PROTOCOL::PlayerServerState::SERVER_STATE_CONNECTED;

}

void ClientSession::OnDisconnected()
{

	cout << "Player-" << _player->_info.objectid() << ", Disconnect" << endl;
	//shared_ptr<Room> room = _player->_ownerRoom;
	
	// �÷��̾ ������
	if (_player != nullptr && _player->_ownerRoom != nullptr) {
		_player->_ownerRoom->DoAsync(&Room::LeaveRoom, _player->_info.objectid());

		// ������Ʈ �Ŵ������� �÷��̾� ����
		ObjectManager::Instance()->Remove(_player->_info.objectid());

		// �÷��̾� ���� ����
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

	//vector<char> _buffer;

}

void ClientSession::SendPacket(shared_ptr<SendBuffer> sendBuffer)
{
	// �� �ɰ�
	lock_guard<mutex> lock(_mutex2);

	// �迭�� Ǫ��
	_packetBuffer.push_back(sendBuffer);
}

void ClientSession::SendPacketFlush()
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
void ClientSession::HandleLogin(PROTOCOL::C_Login fromPkt)
{
	// �α��� Ȯ��
	// ���� (STR -> WSTR)
	string str = fromPkt.id();
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	wstring id(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &id[0], sizeNeeded);

	//
	bool loginResult = false;

	// DB ���� ��ȸ
	AccountDB accountDb;
	DBConnection* dbConn = GDBConnectionPool->Pop();
	SP::SelectAccountByAccountName selectAccount(*dbConn);
	{
		// In
		selectAccount.In_AccountName(id.c_str(), id.size());
		// Out
		selectAccount.Out_AccountDbId(OUT accountDb.AccountDbId);
		selectAccount.Out_AccountName(OUT accountDb.AccountName);
		selectAccount.Out_AccountPw(OUT accountDb.AccountPw);

		// Execute & Fetch
		selectAccount.Execute();
		while (selectAccount.Fetch()) {
			if (accountDb.AccountPw == stoi(fromPkt.pw())) {
				loginResult = true;
				break;
			}
		}
	}
	
	//
	PROTOCOL::S_Login toPkt;

	// �α��� ����
	if (!loginResult) {
		cout << "ID:" << str << " Login Failed" << endl;
		toPkt.set_success(false);
	}
	// �α��� ����
	else {
		toPkt.set_success(true);

		// 
		_accountDbId = accountDb.AccountDbId;
		cout << "ID:" << str << ", AccountDbId:" << _accountDbId << " Login Succeed" << endl;
		
		// DB���� �ش� ������ �÷��̾� ������
		PlayerDB playerDb;
		SP::SelectPlayerByAccountDbId selectPlayer(*dbConn);
		{
			// In
			selectPlayer.In_AccountDbId(_accountDbId);
			// Out
			// �ε���, �̸�, ��������
			selectPlayer.Out_PlayerDbId(playerDb.PlayerDbId);
			selectPlayer.Out_PlayerName(playerDb.PlayerName);
			selectPlayer.Out_AccountDbId(playerDb.AccountDbId);
			
			// ��ġ, ����
			selectPlayer.Out_LocationX(playerDb.LocationX);
			selectPlayer.Out_LocationY(playerDb.LocationY);
			selectPlayer.Out_LocationZ(playerDb.LocationZ);
			selectPlayer.Out_Level(playerDb.Level);
			selectPlayer.Out_TotalExp(playerDb.Exp);
			selectPlayer.Out_MaxHp(playerDb.MaxHp);
			selectPlayer.Out_Hp(playerDb.Hp);
			selectPlayer.Out_Damage(playerDb.Damage);

			// Execute & Fetch
			selectPlayer.Execute();
			while (selectPlayer.Fetch()) {

				//
				PROTOCOL::ObjectInfo info;
				// Ÿ��, �÷��̾� �ε���
				info.set_objecttype(PROTOCOL::GameObjectType::PLAYER);
				info.set_playerdbid(playerDb.PlayerDbId);

				// ĳ���� �̸� (WSTR -> STR)
				wstring wstr(playerDb.PlayerName);
				int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
				string strTo(sizeNeeded, 0);
				WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
				info.set_name(strTo);

				// ��ġ, ����
				info.mutable_pos()->set_locationx(playerDb.LocationX);
				info.mutable_pos()->set_locationy(playerDb.LocationY);
				info.mutable_pos()->set_locationz(playerDb.LocationZ);
				info.mutable_stat()->set_level(playerDb.Level);
				info.mutable_stat()->set_totalexp(DataManager::Instance()->_statTable[playerDb.Level].totalExp);
				info.mutable_stat()->set_exp(playerDb.Exp);
				info.mutable_stat()->set_maxhp(playerDb.MaxHp);
				info.mutable_stat()->set_hp(playerDb.Hp);
				info.mutable_stat()->set_damage(playerDb.Damage);

				// ���� ĳ���� ��Ͽ� �߰� (����)
				_myCharacterList.push_back(info);
				
				// ��Ŷ�� ���� (ĳ���� ���)
				toPkt.add_objectinfos()->CopyFrom(info);	
			}
		}

		// ���� ���� (Connected -> Lobby)
		_playerState = PROTOCOL::PlayerServerState::SERVER_STATE_LOBBY;
	}

	// DB���� �ݳ�
	GDBConnectionPool->Push(dbConn);
	
	// ��Ŷ ����
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	SendPacket(sendBuffer);	
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
	selectPlayer.Execute();

	// �ߺ�
	if (selectPlayer.Fetch() == true)
		toPkt.mutable_object()->set_name("Name Exists");
	
	// �ߺ� X, ��밡��
	else {
		// 1���� ����
		StatData stat = DataManager::Instance()->_statTable[1];

		// DB �÷��̾� ����
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

		SP::InsertPlayer insertPlayer(*dbConn);
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
		if (insertPlayer.Execute() == false) {
			cout << "ClientSession::CreatePlayer() - InsertPlayer Error" << endl;
			return;
		}
		
		// ��� ������ DB�÷��̾�� id ����
		int64 playerDbId;
		SP::SelectIdentity selectIdentity(*dbConn);
		selectIdentity.Out_Identity(playerDbId);
		if (selectIdentity.Execute() == false) {
			cout << "ClientSession::CreatePlayer() - SelectIdentity Error" << endl;
			return;
		}
		if (selectIdentity.Fetch() == false) {
			cout << "ClientSesison::CreatePlayer() - SelectIdentity Error" << endl;
			return;
		}
		playerDB.PlayerDbId = playerDbId;

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

/*
	Ŭ�� ���� C_EnterRoom ��Ŷ���� ���� ���� ĳ����(LobbyPlayerInfo)�� ��ġ�ϴ� ĳ����
	�� ã�� �ش� ĳ���Ͱ� �����ϰ� �ִ� �������� Item DB���� ��ȸ
	
	������ ����� ������� ĳ���� �κ��� �ְ�, Ŭ���̾�Ʈ���� ������ ��� ������
*/
void ClientSession::HandleEnterRoom(PROTOCOL::C_Enter_Room fromPkt)
{
	// ù ����
	if (_player == nullptr) {
		// ������ ��� ��Ŷ
		PROTOCOL::S_ItemList toPkt;

		// DB Ŀ�ؼ�
		DBConnection* dbConn = GDBConnectionPool->Pop();

		// �� ĳ���� ����Ʈ���� ������ ĳ�� �̸� ã��
		for (PROTOCOL::ObjectInfo& info : _myCharacterList) {
			if (info.name().compare(fromPkt.object().name()) == 0) {

				// ������Ʈ ����
				_player = static_pointer_cast<Player>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::PLAYER));

				// id, Ÿ�� ��� ����
				int32 objectId = _player->_info.objectid();
				PROTOCOL::GameObjectType type = _player->_info.objecttype();

				// ������Ʈ�� ������ ����, id,Ÿ�� �ٽ� ����
				_player->_info.CopyFrom(info);
				_player->_info.set_objectid(objectId);
				_player->_info.set_objecttype(type);
				_player->_playerDbId = _player->_info.playerdbid();

				// ���� �÷��̾� ���� (����, �÷��̾� ����)
				_player->_ownerSession = static_pointer_cast<ClientSession>(shared_from_this()); 
				
				// DB ĳ���� ���� ������ ��ȸ
				ItemDB itemDB;
				SP::SelectItemByPlayerDbId selectItem(*dbConn);
				// In
				selectItem.In_PlayerDbId(_player->_info.playerdbid());

				// Out
				selectItem.Out_ItemDbId(itemDB.ItemDbId);
				selectItem.Out_TemplateId(itemDB.TemplateId);
				selectItem.Out_Count(itemDB.Count);
				selectItem.Out_Slot(itemDB.Slot);
				selectItem.Out_Equipped(itemDB.Equipped);
				selectItem.Out_PlayerDbId(itemDB.PlayerDbId);

				// Execute & Fetch
				selectItem.Execute();
				while (selectItem.Fetch()) {
					// ��ȸ�� ������(DB) ���� �޸�(�÷��̾� �κ��丮)���� ����
					shared_ptr<Item> item = Item::MakeItem(itemDB);
					if(item != nullptr)
						_player->_inven.Add(item);

					// ������ ��Ŷ�� �߰�
					toPkt.add_items()->CopyFrom(item->_itemInfo);
				}

				break;
			}
		}

		// DB Ŀ�ؼ� �ݳ�
		GDBConnectionPool->Push(dbConn);

		// ������ ��Ŷ ����
		shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		SendPacket(sendBuffer);
	}

	// ù���� x, �������� ��,
	else {
		// ü��, ������ ���� ����
		_player->_info.mutable_stat()->set_hp(_player->_info.stat().maxhp());
		_player->_info.mutable_pos()->set_locationx(0.f);
		_player->_info.mutable_pos()->set_locationy(-1000.f);
		_player->_info.mutable_pos()->set_locationz(120.f);
	}
	
	// ��(����) ����
	shared_ptr<Room> room = RoomManager::Instance()->Find(1);
	if(room)
		room->DoAsync(&Room::EnterRoom, static_pointer_cast<GameObject>(_player));
}

