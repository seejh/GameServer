#pragma once


/*---------------------------------------------------------------
	���ڵ�
	string token = "~~.~~~.~~~";
	auto decoded = jwt::decode(token);
	for (auto& e : decoded.get_payload_json())
		cout << e.first << ":" << e.second << endl;

	��ū ����
	auto verifier = jwt::verify()
	.with_issuer("")
	.with_claim("", jwt::claim(string("")))
	.allow_algorithm(jwt::algorithm::hs256{"secret"});

	verifier.verify(decoded_token);

	��ū ����
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


*/