#pragma once


#include"DBDataModel.h"

enum { 
	MAX_SLOT_SIZE = 30, 
};

class Item;
class Inventory
{
public:
	// ������ �߰�, ����
	void Add(shared_ptr<Item> item);
	void Remove(int32 slot);

	// ������ Ž��
	shared_ptr<Item> GetItemByItemDbId(int itemDbId);
	shared_ptr<Item> GetItemBySlot(int slot);
	shared_ptr<Item> FindEquippedSamePos(shared_ptr<Item> item);
	
	// ���� Ž��
	// �� ���� Ž��, ������ -1
	int32 GetEmptySlot();
	// ���� ������ ���� Ž��, ������ -1
	int32 GetStackPos(int32 templateId);
	
	// ���� �� (DB�۾� ���� - �ش� ���Կ� ���� DB �۾��� ���ǵ� ���� �ɷ������� ���� �޸���� �����͸� �ŷ��ϸ� �ȵȴ�)
	bool SetSlotDBLock(int32 slot, bool flag);
	
public:
	// <Slot, shared_ptr<Item>>
	map<int, shared_ptr<Item>> _items;
	bool _slotLocker[MAX_SLOT_SIZE + 1];
};

