#include "pch.h"
#include "DBTransaction.h"

#include"GameObject.h"
#include"Room.h"
#include"DataManager.h"
#include"DBConnection.h"
#include"DBConnectionPool.h"
#include"DBDataModel.h"
#include"GenProcedures.h"
#include"Item.h"
#include"Inventory.h"

void DBTransaction::Init()
{
	dbConn = GDBConnectionPool->Pop();

	cout << "DB Transaction Init" << endl;
}

void DBTransaction::RewardPlayer(shared_ptr<Player> player, RewardData reward, shared_ptr<Room> room)
{
	cout << "Player-" << player->_info.objectid() << ", GetRewardItem-" << reward.itemId << endl;

	// 아이템 들어갈 자리 확인
	PROTOCOL::ItemInfo info = player->_inven.GetEmptySlot(reward.itemId);

	// 인벤이 풀
	if (info.slot() == -1) {
		cout << "DBTransaction::RewardPlayer() Error : Inventory Full" << endl;
		return;
	}

	// 새로 생성
	if (info.itemdbid() == -1) {
		// cout << "Create Item New Slot" << endl;

		// 
		info.set_templateid(reward.itemId);
		info.set_count(reward.count);
		// info.set_slot();
		info.set_equipped(false);
		info.set_playerdbid(player->_playerDbId);

		// DB - 생성, 삽입
		SP::InsertItem insertItem(*dbConn);
		insertItem.In_TemplateId(info.templateid());
		insertItem.In_Count(info.count());
		insertItem.In_Slot(info.slot());
		insertItem.In_Equipped(info.equipped());
		insertItem.In_PlayerDbId(info.playerdbid());
		if (insertItem.Execute() == false) {
			cout << "DBTransaction::RewardPlayer() Error : InsertItem Failed" << endl;
			return;
		}
		
		// DB - ID 조회
		int64 outIdentity;
		SP::SelectIdentity selectIdentity(*dbConn);
		selectIdentity.Out_Identity(OUT outIdentity);
		if (selectIdentity.Execute() == false) {
			cout << "DBTransaction::RewardPlayer() Error : SelectIdentity Failed" << endl;
			return;
		}
		selectIdentity.Fetch();

		// 
		info.set_itemdbid(outIdentity);
	}

	// 업데이트
	else {
		// 기존 정보에 카운트만 업데이트
		info.set_count(info.count() + reward.count);

		// DB - 업데이트
		SP::UpdateItem updateItem(*dbConn);
		updateItem.In_ItemDbId(info.itemdbid());
		updateItem.In_TemplateId(info.templateid());
		updateItem.In_Count(info.count());
		updateItem.In_Slot(info.slot());
		updateItem.In_Equipped(info.equipped());
		updateItem.In_PlayerDbId(player->_info.playerdbid());
		if (updateItem.Execute() == false) {
			cout << "DBTransaction::RewardPlayer() Error : UpdateItem Failed" << endl;
			return;
		}
	}

	// 룸에서 아이템 생성 및 패킷
	room->DoAsync(&Room::PlayerAddItem, player, info);
}
