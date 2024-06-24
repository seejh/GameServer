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
	// 인벤토리에서 아이템 추출
	for (auto p : _items) {
		// 아이템
		shared_ptr<Item> nowItem = p.second;
		
		// 본인이면 패스
		if (nowItem->_itemInfo.itemdbid() == item->_itemInfo.itemdbid())
			continue;

		// 같은 타입, 착용중인가
		if (nowItem->_itemType == item->_itemType && nowItem->_itemInfo.equipped() == true) {

			// 방어구
			if (nowItem->_itemType == PROTOCOL::ItemType::ITEM_TYPE_ARMOR) {

				// 방어구 내 동일 포지션
				if (static_pointer_cast<Armor>(nowItem)->_armorType == static_pointer_cast<Armor>(item)->_armorType)
					return nowItem;
			}

			// 무기
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

	// 아이템 정보 조회
	auto it = DataManager::Instance()->_itemTable.find(templateId);
	if (it != DataManager::Instance()->_itemTable.end()) {
	
		// 소모품
		if (it->second->_itemType == PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {
			for (auto p : _items) {
				// 1. 업데이트(스택) - 같은 종의 소모품이 이미 인벤에 있음
				if (p.second->_itemInfo.templateid() == templateId) {

					// 
					info.CopyFrom(p.second->_itemInfo);
					return info;
				}
			}
		}

		// 장비류, 소모품이라도 새로 생성해야하는 상황
		// 슬롯 번호
		for (int slot = 0; slot < 30; slot++) {
			bool use = false;

			// 해당 슬롯번호에 아이템이 있는지
			for (auto p : _items) {
				if (p.second->_itemInfo.slot() == slot) {
					use = true;
					break;
				}
			}

			// 2. 없으면 -> 새로 생성
			if (use == false) {
				info.set_slot(slot);
				return info;
			}
		}
	}

	// 3. 인벤이 풀
	return info;
}
