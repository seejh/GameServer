#include "pch.h"
#include "Item.h"

#include"DataManager.h"

shared_ptr<Item> Item::MakeItem(ItemDB itemDB)
{
	// ������ Ÿ�Կ� ���� ó���� ���� �����͸Ŵ������� ������ ���� ������
	ItemData* itemData = DataManager::Instance()->_itemTable[itemDB.TemplateId];
	if (itemData == nullptr)
		return nullptr;

	// 
	shared_ptr<Item> item;

	// ����
	if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_WEAPON)
		item = static_pointer_cast<Item>(make_shared<Weapon>(itemDB.TemplateId));

	// ��
	else if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR)
		item = static_pointer_cast<Item>(make_shared<Armor>(itemDB.TemplateId));

	// �Ҹ�ǰ
	else if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) 
		item = static_pointer_cast<Item>(make_shared<Consumable>(itemDB.TemplateId));

	// �� ��
	else
		return nullptr;

	// ���� ���� ����
	item->_itemInfo.set_itemdbid(itemDB.ItemDbId);
	item->_itemInfo.set_templateid(itemDB.TemplateId);
	item->_itemInfo.set_count(itemDB.Count);
	item->_itemInfo.set_slot(itemDB.Slot);
	item->_itemInfo.set_equipped(itemDB.Equipped);
	item->_itemInfo.set_playerdbid(itemDB.PlayerDbId);
	
	// 
	return item;
}
