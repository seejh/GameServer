#pragma once


/*---------------------------------------------------------------
	디코드
	string token = "~~.~~~.~~~";
	auto decoded = jwt::decode(token);
	for (auto& e : decoded.get_payload_json())
		cout << e.first << ":" << e.second << endl;

	토큰 검증
	auto verifier = jwt::verify()
	.with_issuer("")
	.with_claim("", jwt::claim(string("")))
	.allow_algorithm(jwt::algorithm::hs256{"secret"});

	verifier.verify(decoded_token);

	토큰 생성
	auto token = jwt::create()
	.set_type("JWS")
	.set_issuer("")
	.set_payload_cliam("", jwt::claim(string("")))
	.sign(jwt::algorithm::hs256 {"secret"});
---------------------------------------------------------------*/

class JwtHandler
{
public:
	static bool VerifyToken(string token);
	static bool MakeCompare(string token);
};


/*
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


*/