#include "pch.h"
#include "Inventory.h"

#include"Item.h"

void Inventory::Add(shared_ptr<Item> item)
{
	_items[item->_itemInfo.slot()] = item;
}

void Inventory::Remove(int32 slot)
{
	_items.erase(slot);
}

shared_ptr<Item> Inventory::GetItemByItemDbId(int itemDbId)
{
	auto it = _items.find(itemDbId);
	if (it != _items.end())
		return it->second;

	return nullptr;
}

shared_ptr<Item> Inventory::GetItemBySlot(int slot)
{
	if (slot > MAX_SLOT_SIZE)
		return nullptr;

	return _items[slot];
}

shared_ptr<Item> Inventory::FindEquippedSamePos(shared_ptr<Item> item)
{
	// ���� �κ��丮�� �ִ� �����۵鿡�� ��ȸ
	for (auto p : _items) {
		// 
		shared_ptr<Item>& nowItem = p.second;
		
		// �����̸� �н�
		if (nowItem->_itemInfo.itemdbid() == item->_itemInfo.itemdbid())
			continue;

		// ���� Ÿ��, ������
		if (nowItem->_itemType == item->_itemType && nowItem->_itemInfo.equipped() == true) {

			// ����� �� �� ���� ������(Ex.���, ����) ���� Ȯ��
			if (nowItem->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR) {
				if (static_pointer_cast<Armor>(nowItem)->_armorType == static_pointer_cast<Armor>(item)->_armorType)
					return nowItem;
			}

			// ����
			else
				return nowItem;
		}
	}

	// ���� �����ǿ� �������� ��� ����
	return nullptr;
}

int32 Inventory::GetEmptySlot()
{
	for (int i = 0; i < MAX_SLOT_SIZE; i++) {
		if (_items[i] == nullptr && _slotLocker[i] == false) 
			return i;
	}

	return -1;
}

int32 Inventory::GetStackPos(int32 templateId)
{
	// �����ΰ�
	ItemData* itemData = DataManager::Instance()->_itemTable[templateId];
	if (itemData && itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
		// ���� �κ��� �������ΰ� -> ���ð���
		for (auto p : _items) {
			if (p.second != nullptr && p.second->_itemInfo.templateid() == templateId)
				return p.second->_itemInfo.slot();
		}
	}

	// ���� �Ұ���
	return -1;
}

bool Inventory::SetSlotDBLock(int32 slot, bool flag)
{
	// ���µ� ����Ϸ��� �ϸ�
	if (flag == true && _slotLocker[slot] == true) {
		// cout << "[INVENTORY] Slot:" << slot << "Try Locking, But Already Locked" << endl;
		return false;
	}

	/*if (flag == true)
		cout << "[INVENTORY] Slot: " << slot << " Lock OK" << endl;
	else
		cout << "[INVENTORY] Slot: " << slot << "Lock Release OK" << endl;*/

	_slotLocker[slot] = flag;

	return true;
}


