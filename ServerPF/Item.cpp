#include "pch.h"
#include "Item.h"

#include"DataManager.h"

shared_ptr<Item> Item::MakeItem(ItemDB itemDB)
{
	// ������
	shared_ptr<Item> item;
	
	// ������ Ÿ�Կ� ���� ó���� ���� �����͸Ŵ������� ������ ���� ������
	ItemData* itemData = DataManager::Instance()->_itemTable[itemDB.TemplateId];
	if (itemData == nullptr)
		return nullptr;

	// ����
	if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_WEAPON) {
		// ���� Ÿ�� �ν��Ͻ� ����
		item = static_pointer_cast<Item>(make_shared<Weapon>(itemDB.TemplateId));
	}
	// ��
	else if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR) {
		// �� Ÿ�� �ν��Ͻ� ����
		item = static_pointer_cast<Item>(make_shared<Armor>(itemDB.TemplateId));
	}
	// �Һ��
	else if (itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
		// �Һ� Ÿ�� �ν��Ͻ� ����
		item = static_pointer_cast<Item>(make_shared<Consumable>(itemDB.TemplateId));
	}
	// �� ��
	else {
		// �ϴ� NONE
		return nullptr;
	}

	// ���� ���� ����
	item->_itemInfo.set_itemdbid(itemDB.ItemDbId);
	item->_itemInfo.set_templateid(itemDB.TemplateId);
	item->_itemInfo.set_count(itemDB.Count);
	item->_itemInfo.set_slot(itemDB.Slot);
	item->_itemInfo.set_equipped(itemDB.Equipped);
	
	// ������ ����
	return item;
}
