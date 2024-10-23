#pragma once

#include"DBDataModel.h"

/*-----------------------------------------------------------------------
	Item 
	소지정보만 가지고 스펙은 데이터매니저에서 조회하는 식으로, 다만,
	간략한 정보는 들고 있는 듯

	스펙 정보 - DataContents.h에서 ItemData 객체로 다룸
	json파일에 담겨 있는 스펙을 가져옴
	id, name, itemtype, 세부정보(ex.weapontype, damage)
	
	소지 정보 - DBDataModel.h에서 ItemDB 클래스 타입으로 관리
	DB에 저장되는 데이터 구조
	itemdbid, templateid, count, slot, equipped

-----------------------------------------------------------------------*/
enum ITEM_STATE {
	ITEM_STATE_NONE, ITEM_STATE_UPDATE, ITEM_STATE_DELETE, ITEM_STATE_MAKENEW, ITEM_STATE_NOCHANGE,
};

class Item : public enable_shared_from_this<Item>
{
public:
	Item(PROTOCOL::ItemType itemType) : _itemType(itemType) {}

	// 아이템 생성 (소지 정보 + 아이템 템플릿 넘버 기반으로)
	static shared_ptr<Item> MakeItem(ItemDB itemDB);

public:
	// 소지 정보
	PROTOCOL::ItemInfo _itemInfo;

	// 스펙 정보 (공용) - 아이템 타입, 중복 여부
	PROTOCOL::ItemType _itemType; 
	bool _stackable;
};

// 무기 아이템
class Weapon : public Item {
public:
	Weapon(int templateId) : Item(PROTOCOL::ITEM_TYPE_WEAPON) { 
		Init(templateId); 
	}
	
	// 아이템 스펙 설정
	void Init(int templateId) {
		// 데이터매니저에서 아이템 스펙 조회
		WeaponData* weaponData = reinterpret_cast<WeaponData*>(DataManager::Instance()->_itemTable[templateId]);
		
		// 스펙 설정
		_weaponType = weaponData->_weaponType;
		_damage = weaponData->_damage;
		_stackable = false;
	}
public:
	// 스펙 정보 (상세) - 무기타입, 공격력
	PROTOCOL::WeaponType _weaponType;
	int _damage;
};

// 방어구 아이템
class Armor : public Item {
public:
	Armor(int templateId) : Item(PROTOCOL::ITEM_TYPE_ARMOR) { 
		Init(templateId); 
	}
	
	// 아이템 스펙 설정
	void Init(int templateId) {
		// 데이터매니저에서 아이템 스펙 조회
		ArmorData* armorData = reinterpret_cast<ArmorData*>(DataManager::Instance()->_itemTable[templateId]);
		
		// 스펙 설정
		_armorType = armorData->_armorType;
		_defence = armorData->_defence;
		_stackable = false;
	}
public:
	// 스펙 정보 (상세) - 방어구 타입, 방어력
	PROTOCOL::ArmorType _armorType;
	int _defence;
};

// 소모류 아이템
class Consumable : public Item {
public:
	Consumable(int templateId) : Item(PROTOCOL::ITEM_TYPE_CONSUMABLE) { 
		Init(templateId); 
	}
	
	// 아이템 스펙 설정
	void Init(int templateId) {
		// 데이터매니저에서 아이템 스펙 조회
		ConsumableData* consumableData = reinterpret_cast<ConsumableData*>(DataManager::Instance()->_itemTable[templateId]);
		
		// 스펙 설정
		_consumableType = consumableData->_consumableType;
		_maxCount = consumableData->_maxCount;
		_stackable = consumableData->_maxCount > 1;
	}
public:
	// 스펙 정보 (상세) - 소모류 타입, 최대개수
	PROTOCOL::ConsumableType _consumableType;
	int _maxCount;
};