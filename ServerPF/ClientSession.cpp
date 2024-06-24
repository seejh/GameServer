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
	
	// 플레이어가 있으면
	if (_player != nullptr && _player->_ownerRoom != nullptr) {
		_player->_ownerRoom->DoAsync(&Room::LeaveRoom, _player->_info.objectid());

		// 오브젝트 매니저에서 플레이어 제거
		ObjectManager::Instance()->Remove(_player->_info.objectid());

		// 플레이어 참조 해제
		_player.reset();
	}

	// 세션 매니저에서 이 세션 제거
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
	// 락 걸고
	lock_guard<mutex> lock(_mutex2);

	// 배열에 푸시
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
	// 로그인 확인
	// 계정 (STR -> WSTR)
	string str = fromPkt.id();
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	wstring id(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &id[0], sizeNeeded);

	//
	bool loginResult = false;

	// DB 계정 조회
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

	// 로그인 실패
	if (!loginResult) {
		cout << "ID:" << str << " Login Failed" << endl;
		toPkt.set_success(false);
	}
	// 로그인 성공
	else {
		toPkt.set_success(true);

		// 
		_accountDbId = accountDb.AccountDbId;
		cout << "ID:" << str << ", AccountDbId:" << _accountDbId << " Login Succeed" << endl;
		
		// DB에서 해당 계정의 플레이어 정보들
		PlayerDB playerDb;
		SP::SelectPlayerByAccountDbId selectPlayer(*dbConn);
		{
			// In
			selectPlayer.In_AccountDbId(_accountDbId);
			// Out
			// 인덱스, 이름, 소유계정
			selectPlayer.Out_PlayerDbId(playerDb.PlayerDbId);
			selectPlayer.Out_PlayerName(playerDb.PlayerName);
			selectPlayer.Out_AccountDbId(playerDb.AccountDbId);
			
			// 위치, 스텟
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
				// 타입, 플레이어 인덱스
				info.set_objecttype(PROTOCOL::GameObjectType::PLAYER);
				info.set_playerdbid(playerDb.PlayerDbId);

				// 캐릭터 이름 (WSTR -> STR)
				wstring wstr(playerDb.PlayerName);
				int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
				string strTo(sizeNeeded, 0);
				WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
				info.set_name(strTo);

				// 위치, 스텟
				info.mutable_pos()->set_locationx(playerDb.LocationX);
				info.mutable_pos()->set_locationy(playerDb.LocationY);
				info.mutable_pos()->set_locationz(playerDb.LocationZ);
				info.mutable_stat()->set_level(playerDb.Level);
				info.mutable_stat()->set_totalexp(DataManager::Instance()->_statTable[playerDb.Level].totalExp);
				info.mutable_stat()->set_exp(playerDb.Exp);
				info.mutable_stat()->set_maxhp(playerDb.MaxHp);
				info.mutable_stat()->set_hp(playerDb.Hp);
				info.mutable_stat()->set_damage(playerDb.Damage);

				// 소유 캐릭터 목록에 추가 (서버)
				_myCharacterList.push_back(info);
				
				// 패킷에 복사 (캐릭터 목록)
				toPkt.add_objectinfos()->CopyFrom(info);	
			}
		}

		// 상태 변경 (Connected -> Lobby)
		_playerState = PROTOCOL::PlayerServerState::SERVER_STATE_LOBBY;
	}

	// DB연결 반납
	GDBConnectionPool->Push(dbConn);
	
	// 패킷 전송
	shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	SendPacket(sendBuffer);	
}

void ClientSession::HandleCreatePlayer(string name)
{
	// TODO : 로비에서만 생성 가능하게

	// 패킷
	PROTOCOL::S_CreatePlayer toPkt;

	// DB 커넥션
	DBConnection* dbConn = GDBConnectionPool->Pop();

	// 캐릭터 이름 (STR -> WSTR) 
	int needSize = MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), NULL, 0);
	wstring wstr(needSize, 0);
	MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), &wstr[0], needSize);
	wstr.append(L"\0");
	
	// 아이디 중복 확인
	int32 num;
	SP::SelectPlayerName selectPlayer(*dbConn);
	selectPlayer.In_PlayerName(wstr.c_str(), wstr.size());
	selectPlayer.Out_PlayerDbId(num);
	selectPlayer.Execute();

	// 중복
	if (selectPlayer.Fetch() == true)
		toPkt.mutable_object()->set_name("Name Exists");
	
	// 중복 X, 사용가능
	else {
		// 1레벨 스텟
		StatData stat = DataManager::Instance()->_statTable[1];

		// DB 플레이어 생성
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
		
		// 방금 생성한 DB플레이어에서 id 추출
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

		// 기본 아이템 지급
		// 기본 아이템 - 검
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

		// 기본 아이템 - 포션
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

		// (서버) 이 계정 보유 캐릭터 목록 추가
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

		// 패킷에 새 캐릭터 정보 추가
		toPkt.mutable_object()->CopyFrom(newCharacterInfo);
	}

	// 전송
	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
	SendPacket(sendBuffer);

	// DB 반납
	GDBConnectionPool->Push(dbConn);
}

/*
	클라가 보낸 C_EnterRoom 패킷에서 현재 보유 캐릭터(LobbyPlayerInfo)와 일치하는 캐릭터
	를 찾고 해당 캐릭터가 소유하고 있는 아이템을 Item DB에서 조회
	
	아이템 목록을 만들어준 캐릭터 인벤에 넣고, 클라이언트에게 아이템 목록 보내줌
*/
void ClientSession::HandleEnterRoom(PROTOCOL::C_Enter_Room fromPkt)
{
	// 첫 접속
	if (_player == nullptr) {
		// 아이템 목록 패킷
		PROTOCOL::S_ItemList toPkt;

		// DB 커넥션
		DBConnection* dbConn = GDBConnectionPool->Pop();

		// 내 캐릭터 리스트에서 접속할 캐릭 이름 찾음
		for (PROTOCOL::ObjectInfo& info : _myCharacterList) {
			if (info.name().compare(fromPkt.object().name()) == 0) {

				// 오브젝트 생성
				_player = static_pointer_cast<Player>(ObjectManager::Instance()->Add(PROTOCOL::GameObjectType::PLAYER));

				// id, 타입 잠시 보관
				int32 objectId = _player->_info.objectid();
				PROTOCOL::GameObjectType type = _player->_info.objecttype();

				// 오브젝트에 데이터 복사, id,타입 다시 넣음
				_player->_info.CopyFrom(info);
				_player->_info.set_objectid(objectId);
				_player->_info.set_objecttype(type);
				_player->_playerDbId = _player->_info.playerdbid();

				// 접속 플레이어 설정 (세션, 플레이어 참조)
				_player->_ownerSession = static_pointer_cast<ClientSession>(shared_from_this()); 
				
				// DB 캐릭터 소유 아이템 조회
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
					// 조회된 아이템(DB) 서버 메모리(플레이어 인벤토리)에서 생성
					shared_ptr<Item> item = Item::MakeItem(itemDB);
					if(item != nullptr)
						_player->_inven.Add(item);

					// 아이템 패킷에 추가
					toPkt.add_items()->CopyFrom(item->_itemInfo);
				}

				break;
			}
		}

		// DB 커넥션 반납
		GDBConnectionPool->Push(dbConn);

		// 아이템 패킷 전송
		shared_ptr<SendBuffer> sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		SendPacket(sendBuffer);
	}

	// 첫접속 x, 리스폰일 시,
	else {
		// 체력, 리스폰 지점 조정
		_player->_info.mutable_stat()->set_hp(_player->_info.stat().maxhp());
		_player->_info.mutable_pos()->set_locationx(0.f);
		_player->_info.mutable_pos()->set_locationy(-1000.f);
		_player->_info.mutable_pos()->set_locationz(120.f);
	}
	
	// 룸(월드) 입장
	shared_ptr<Room> room = RoomManager::Instance()->Find(1);
	if(room)
		room->DoAsync(&Room::EnterRoom, static_pointer_cast<GameObject>(_player));
}

