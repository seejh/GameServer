#pragma once



/*--------------------------------------------------------
	Monster, Monsters Reward 정보
--------------------------------------------------------*/
class RewardData {
public:
	RewardData() : itemId(0) {}
public:
	float itemDropRate;
	int32 itemId;
	int32 count;
};

class MonsterData {
public:
	int32 id;
	string name;
	PROTOCOL::Stat stat;
	vector<RewardData> rewardDatas;
};


/*--------------------------------------------------------
	Item 정보
--------------------------------------------------------*/
class ItemData {
public:
	ItemData() {}
	ItemData(PROTOCOL::ItemType itemType) : _itemType(itemType) {}
public:
	int32 _id;
	string _name;
	PROTOCOL::ItemType _itemType;
};

class WeaponData : public ItemData {
public:
	WeaponData() : ItemData(PROTOCOL::ItemType::ITEM_TYPE_WEAPON) {}
public:
	PROTOCOL::WeaponType _weaponType;
	int _damage;
};

class ArmorData : public ItemData {
public:
	ArmorData() : ItemData(PROTOCOL::ItemType::ITEM_TYPE_ARMOR) {}
public:
	PROTOCOL::ArmorType _armorType;
	int _defence;
};

class ConsumableData : public ItemData {
public:
	ConsumableData() : ItemData(PROTOCOL::ItemType::ITEM_TYPE_CONSUMABLE) {}
public:
	PROTOCOL::ConsumableType _consumableType;
	int _maxCount;
	int _recovery;
};

/*-------------------------------------------------
	SKill 정보
-------------------------------------------------*/
class SkillData {
public:
	int id;
	string name;
	float cooltime;
	int damage;
	PROTOCOL::SkillType skillType;
};

/*--------------------------------------------------------
	Stat 정보
--------------------------------------------------------*/
class StatData {
public:
	int level;
	int totalExp;
	int maxhp;
	int hp;
	int damage;
	float speed;
	float attackDistance;
	float noticeDistance;
	float returnDistance;
	float attackCoolTime;
};
