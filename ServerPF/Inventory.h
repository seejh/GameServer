#pragma once


/*
아이템을 획득할 때
DB에 먼저 넣어주고 그 뒤에 서버 플레이어 인벤에 넣어주는데 이거 때문에
들어갈 위치(슬롯)부터 조회해야댐

현재 문제가 아이템 타입에 따라, 슬롯 조회가 여러번 일어난다면, 등의 문제가 있다.
1번은 쉽게 가능, 2번이 빡샘
*/
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
	
	
	// 빈슬롯의 인덱스를 리턴, 공간이 없으면 -1
	int32 GetEmptySlot();
	// 스택가능한 슬롯 리턴, 불가능하면 -1
	int32 GetStackPos(int32 templateId);

	// 슬롯 락 (DB작업 방지 - 해당 슬롯에 대해 DB 작업이 들어갈건데 락이 걸려있으면 서버 메모상의 데이터를 신뢰하면 안된다)
	bool SetSlotDBLock(int32 slot, bool flag);
	
public:
	// <Slot, shared_ptr<Item>>
	map<int, shared_ptr<Item>> _items;
	bool _slotLocker[MAX_SLOT_SIZE + 1];
};

