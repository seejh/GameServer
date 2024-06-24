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

	// ������ �� �ڸ� Ȯ��
	PROTOCOL::ItemInfo info = player->_inven.GetEmptySlot(reward.itemId);

	// �κ��� Ǯ
	if (info.slot() == -1) {
		cout << "DBTransaction::RewardPlayer() Error : Inventory Full" << endl;
		return;
	}

	// ���� ����
	if (info.itemdbid() == -1) {
		// cout << "Create Item New Slot" << endl;

		// 
		info.set_templateid(reward.itemId);
		info.set_count(reward.count);
		// info.set_slot();
		info.set_equipped(false);
		info.set_playerdbid(player->_playerDbId);

		// DB - ����, ����
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
		
		// DB - ID ��ȸ
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

	// ������Ʈ
	else {
		// ���� ������ ī��Ʈ�� ������Ʈ
		info.set_count(info.count() + reward.count);

		// DB - ������Ʈ
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

	// �뿡�� ������ ���� �� ��Ŷ
	room->DoAsync(&Room::PlayerAddItem, player, info);
}
