#pragma once


#include"DBDataModel.h"

enum { 
	MAX_SLOT_SIZE = 30, 
};

class Item;
class Inventory
{
public:
	// 아이템 추가, 삭제
	void Add(shared_ptr<Item> item);
	void Remove(int32 slot);

	// 아이템 탐색
	shared_ptr<Item> GetItemByItemDbId(int itemDbId);
	shared_ptr<Item> GetItemBySlot(int slot);
	shared_ptr<Item> FindEquippedSamePos(shared_ptr<Item> item);
	
	// 슬롯 탐색
	// 빈 슬롯 탐색, 없으면 -1
	int32 GetEmptySlot();
	// 스택 가능한 슬롯 탐색, 없으면 -1
	int32 GetStackPos(int32 templateId);
	
	// 슬롯 락 (DB작업 방지 - 해당 슬롯에 대해 DB 작업이 들어갈건데 락이 걸려있으면 서버 메모상의 데이터를 신뢰하면 안된다)
	bool SetSlotDBLock(int32 slot, bool flag);
	
public:
	// <Slot, shared_ptr<Item>>
	map<int, shared_ptr<Item>> _items;
	bool _slotLocker[MAX_SLOT_SIZE + 1];
};

