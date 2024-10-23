#include "pch.h"
#include "Item.h"

#include"DataManager.h"

shared_ptr<Item> Item::MakeItem(ItemDB itemDB)
{
	// 아이템 타입에 따른 처리를 위해 데이터매니저에서 아이템 스펙 가져옴
	ItemData* itemData = DataManager::Instance()->_itemTable[itemDB.TemplateId];
	if (itemData == nullptr)
		return nullptr;

	// 
	shared_ptr<Item> item;

	// 무기
	if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_WEAPON)
		item = static_pointer_cast<Item>(make_shared<Weapon>(itemDB.TemplateId));

	// 방어구
	else if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR)
		item = static_pointer_cast<Item>(make_shared<Armor>(itemDB.TemplateId));

	// 소모품
	else if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) 
		item = static_pointer_cast<Item>(make_shared<Consumable>(itemDB.TemplateId));

	// 그 외
	else
		return nullptr;

	// 소지 정보 설정
	item->_itemInfo.set_itemdbid(itemDB.ItemDbId);
	item->_itemInfo.set_templateid(itemDB.TemplateId);
	item->_itemInfo.set_count(itemDB.Count);
	item->_itemInfo.set_slot(itemDB.Slot);
	item->_itemInfo.set_equipped(itemDB.Equipped);
	item->_itemInfo.set_playerdbid(itemDB.PlayerDbId);
	
	// 
	return item;
}
