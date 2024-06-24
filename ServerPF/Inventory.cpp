#include "pch.h"
#include "Inventory.h"

#include"Item.h"

void Inventory::Add(shared_ptr<Item> item)
{
	_items[item->_itemInfo.itemdbid()] = item;
}

void Inventory::Remove(int32 itemDbId)
{
	_items.erase(itemDbId);
}

shared_ptr<Item> Inventory::Get(int itemDbId)
{
	auto it = _items.find(itemDbId);
	if (it != _items.end())
		return it->second;

	return nullptr;
}

shared_ptr<Item> Inventory::FindEquipped(shared_ptr<Item> item)
{
	// �κ��丮���� ������ ����
	for (auto p : _items) {
		// ������
		shared_ptr<Item> nowItem = p.second;
		
		// �����̸� �н�
		if (nowItem->_itemInfo.itemdbid() == item->_itemInfo.itemdbid())
			continue;

		// ���� Ÿ��, �������ΰ�
		if (nowItem->_itemType == item->_itemType && nowItem->_itemInfo.equipped() == true) {

			// ��
			if (nowItem->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR) {

				// �� �� ���� ������
				if (static_pointer_cast<Armor>(nowItem)->_armorType == static_pointer_cast<Armor>(item)->_armorType)
					return nowItem;
			}

			// ����
			else
				return nowItem;
		}
	}

	// 
	return nullptr;
}

//int Inventory::GetEmptySlot()
//{
//	for (int slot = 0; slot < 20; slot++) {
//		bool use = false;
//
//		for (auto p : _items) {
//			if (p.second->_itemInfo.slot() == slot) {
//				use = true;
//				break;
//			}
//		}
//
//		if (use == false)
//			return slot;
//	}
//
//	return -1;
//}

PROTOCOL::ItemInfo Inventory::GetEmptySlot(int templateId)
{
	//
	PROTOCOL::ItemInfo info;
	info.set_itemdbid(-1);

	// ������ ���� ��ȸ
	auto it = DataManager::Instance()->_itemTable.find(templateId);
	if (it != DataManager::Instance()->_itemTable.end()) {
	
		// �Ҹ�ǰ
		if (it->second->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
			for (auto p : _items) {
				// 1. ������Ʈ(����) - ���� ���� �Ҹ�ǰ�� �̹� �κ��� ����
				if (p.second->_itemInfo.templateid() == templateId) {

					// 
					info.CopyFrom(p.second->_itemInfo);
					return info;
				}
			}
		}

		// ����, �Ҹ�ǰ�̶� ���� �����ؾ��ϴ� ��Ȳ
		// ���� ��ȣ
		for (int slot = 0; slot < 30; slot++) {
			bool use = false;

			// �ش� ���Թ�ȣ�� �������� �ִ���
			for (auto p : _items) {
				if (p.second->_itemInfo.slot() == slot) {
					use = true;
					break;
				}
			}

			// 2. ������ -> ���� ����
			if (use == false) {
				info.set_slot(slot);
				return info;
			}
		}
	}

	// 3. �κ��� Ǯ
	return info;
}
