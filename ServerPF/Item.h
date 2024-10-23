#pragma once

#include"DBDataModel.h"

/*-----------------------------------------------------------------------
	Item 
	���������� ������ ������ �����͸Ŵ������� ��ȸ�ϴ� ������, �ٸ�,
	������ ������ ��� �ִ� ��

	���� ���� - DataContents.h���� ItemData ��ü�� �ٷ�
	json���Ͽ� ��� �ִ� ������ ������
	id, name, itemtype, ��������(ex.weapontype, damage)
	
	���� ���� - DBDataModel.h���� ItemDB Ŭ���� Ÿ������ ����
	DB�� ����Ǵ� ������ ����
	itemdbid, templateid, count, slot, equipped

-----------------------------------------------------------------------*/
enum ITEM_STATE {
	ITEM_STATE_NONE, ITEM_STATE_UPDATE, ITEM_STATE_DELETE, ITEM_STATE_MAKENEW, ITEM_STATE_NOCHANGE,
};

class Item : public enable_shared_from_this<Item>
{
public:
	Item(PROTOCOL::ItemType itemType) : _itemType(itemType) {}

	// ������ ���� (���� ���� + ������ ���ø� �ѹ� �������)
	static shared_ptr<Item> MakeItem(ItemDB itemDB);

public:
	// ���� ����
	PROTOCOL::ItemInfo _itemInfo;

	// ���� ���� (����) - ������ Ÿ��, �ߺ� ����
	PROTOCOL::ItemType _itemType; 
	bool _stackable;
};

// ���� ������
class Weapon : public Item {
public:
	Weapon(int templateId) : Item(PROTOCOL::ITEM_TYPE_WEAPON) { 
		Init(templateId); 
	}
	
	// ������ ���� ����
	void Init(int templateId) {
		// �����͸Ŵ������� ������ ���� ��ȸ
		WeaponData* weaponData = reinterpret_cast<WeaponData*>(DataManager::Instance()->_itemTable[templateId]);
		
		// ���� ����
		_weaponType = weaponData->_weaponType;
		_damage = weaponData->_damage;
		_stackable = false;
	}
public:
	// ���� ���� (��) - ����Ÿ��, ���ݷ�
	PROTOCOL::WeaponType _weaponType;
	int _damage;
};

// �� ������
class Armor : public Item {
public:
	Armor(int templateId) : Item(PROTOCOL::ITEM_TYPE_ARMOR) { 
		Init(templateId); 
	}
	
	// ������ ���� ����
	void Init(int templateId) {
		// �����͸Ŵ������� ������ ���� ��ȸ
		ArmorData* armorData = reinterpret_cast<ArmorData*>(DataManager::Instance()->_itemTable[templateId]);
		
		// ���� ����
		_armorType = armorData->_armorType;
		_defence = armorData->_defence;
		_stackable = false;
	}
public:
	// ���� ���� (��) - �� Ÿ��, ����
	PROTOCOL::ArmorType _armorType;
	int _defence;
};

// �Ҹ�� ������
class Consumable : public Item {
public:
	Consumable(int templateId) : Item(PROTOCOL::ITEM_TYPE_CONSUMABLE) { 
		Init(templateId); 
	}
	
	// ������ ���� ����
	void Init(int templateId) {
		// �����͸Ŵ������� ������ ���� ��ȸ
		ConsumableData* consumableData = reinterpret_cast<ConsumableData*>(DataManager::Instance()->_itemTable[templateId]);
		
		// ���� ����
		_consumableType = consumableData->_consumableType;
		_maxCount = consumableData->_maxCount;
		_stackable = consumableData->_maxCount > 1;
	}
public:
	// ���� ���� (��) - �Ҹ�� Ÿ��, �ִ밳��
	PROTOCOL::ConsumableType _consumableType;
	int _maxCount;
};