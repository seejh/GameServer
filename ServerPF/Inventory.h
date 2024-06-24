#pragma once



class Item;
class Inventory
{
public:
	// ItemDB(DB)에서 조회하고 서버 메모리(인벤토리)로 올릴 때
	void Add(shared_ptr<Item> item);
	void Remove(int32 itemDbId);

	// 인벤토리 아이템을 ItemDbId로 가져옴
	shared_ptr<Item> Get(int itemDbId);

	// 조건에 맞는 아이템 찾음
	shared_ptr<Item> FindEquipped(shared_ptr<Item> item);
	
	// 빈슬롯 조회
	PROTOCOL::ItemInfo GetEmptySlot(int templateId);
	//int GetEmptySlot();
	//int GetEmptySlot(int templateId, ItemDB& itemDB);
public:
	// <ItemDbId, shared_ptr<Item>>
	map<int, shared_ptr<Item>> _items; 
};

