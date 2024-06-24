#pragma once



class Item;
class Inventory
{
public:
	// ItemDB(DB)���� ��ȸ�ϰ� ���� �޸�(�κ��丮)�� �ø� ��
	void Add(shared_ptr<Item> item);
	void Remove(int32 itemDbId);

	// �κ��丮 �������� ItemDbId�� ������
	shared_ptr<Item> Get(int itemDbId);

	// ���ǿ� �´� ������ ã��
	shared_ptr<Item> FindEquipped(shared_ptr<Item> item);
	
	// �󽽷� ��ȸ
	PROTOCOL::ItemInfo GetEmptySlot(int templateId);
	//int GetEmptySlot();
	//int GetEmptySlot(int templateId, ItemDB& itemDB);
public:
	// <ItemDbId, shared_ptr<Item>>
	map<int, shared_ptr<Item>> _items; 
};

