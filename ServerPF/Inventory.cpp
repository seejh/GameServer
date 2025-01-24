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
	// 현재 인벤토리에 있는 아이템들에서 조회
	for (auto p : _items) {
		// 
		shared_ptr<Item>& nowItem = p.second;
		
		// 본인이면 패스
		if (nowItem->_itemInfo.itemdbid() == item->_itemInfo.itemdbid())
			continue;

		// 같은 타입, 착용중
		if (nowItem->_itemType == item->_itemType && nowItem->_itemInfo.equipped() == true) {

			// 방어구라면 방어구 중 동일 포지션(Ex.헬멧, 갑옷) 인지 확인
			if (nowItem->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR) {
				if (static_pointer_cast<Armor>(nowItem)->_armorType == static_pointer_cast<Armor>(item)->_armorType)
					return nowItem;
			}

			// 무기
			else
				return nowItem;
		}
	}

	// 동일 포지션에 착용중인 장비 없음
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
	// 포션인가
	ItemData* itemData = DataManager::Instance()->_itemTable[templateId];
	if (itemData && itemData->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
		// 현재 인벤을 점유중인가 -> 스택가능
		for (auto p : _items) {
			if (p.second != nullptr && p.second->_itemInfo.templateid() == templateId)
				return p.second->_itemInfo.slot();
		}
	}

	// 스택 불가능
	return -1;
}

bool Inventory::SetSlotDBLock(int32 slot, bool flag)
{
	// 잠겼는데 잠금하려고 하면
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


