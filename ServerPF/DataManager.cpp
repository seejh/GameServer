#include "pch.h"
#include "DataManager.h"

#include<fstream>
#include"DataContents.h"


bool DataManager::Init()
{
	if (LoadMonsterTable() == false) {
		cout << "LoadMonsterTable Error" << endl;
		return false;
	}
	if (LoadItemTable() == false) {
		cout << "LoadItemTable Error" << endl;
		return false;
	}
	if (LoadStatTable() == false) {
		cout << "LoadStatTable Error" << endl;
		return false;
	}
	if (LoadSkillTable() == false) {
		cout << "LoadSkillTable Error" << endl;
		return false;
	}

	cout << "DataManager Load Data OK" << endl;

	return true;
}

bool DataManager::OpenAndParseJson(string path, rapidjson::Document& doc)
{
	ifstream ifs(path.c_str());

	// Not Open Error
	if (!ifs.is_open()) {
		cout << "Open Failed : " << path << endl;
		return false;
	}

	string jsonString;
	for (char c; ifs >> c;)
		jsonString += c;

	doc.Parse(jsonString.c_str());
	if (doc.HasParseError()) {
		cout << "Parsing Error : " << path << ", " << doc.GetParseError() << endl;
		return false;
	}

	return true;
}

// ����
bool DataManager::LoadStatTable()
{
	// ���� ���� �Ľ�
	rapidjson::Document doc;
	if (!OpenAndParseJson("..\\Common\\Data\\StatData.json", doc))
		return false;

	// ���� ������ �ε�
	const rapidjson::Value& v = doc["stats"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		
		//
		StatData statData;

		// ����
		statData.level = (*it)["level"].GetInt();
		statData.maxhp = (*it)["maxHp"].GetInt();
		statData.damage = (*it)["damage"].GetInt();
		statData.totalExp = (*it)["totalExp"].GetInt();

		// ���� ���̺� ���� (������ �з�)
		_statTable[statData.level] = statData;
	}

	return true;
}

// ������
bool DataManager::LoadItemTable()
{
	// ���� ���� �Ľ�
	rapidjson::Document doc;
	if (!OpenAndParseJson("..\\Common\\Data\\ItemData.json", doc))
		return false;

	// ����
	const rapidjson::Value& v = doc["weapons"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		// 
		WeaponData* weaponData = new WeaponData();

		// id, ��Ī, ���ݷ�
		weaponData->_id = (*it)["id"].GetInt();
		weaponData->_name = (*it)["name"].GetString();
		weaponData->_damage = (*it)["damage"].GetInt();
		
		// ���� Ÿ��
		string weaponType = (*it)["weaponType"].GetString();
		if (weaponType.compare("Sword") == 0) 
			weaponData->_weaponType = PROTOCOL::WeaponType::WEAPON_TYPE_SWORD;
		else if (weaponType.compare("Axe") == 0) 
			weaponData->_weaponType = PROTOCOL::WeaponType::WEAPON_TYPE_AXE;
		else 
			weaponData->_weaponType = PROTOCOL::WeaponType::WEAPON_TYPE_NONE;
		
		// ������ ���̺� ����
		_itemTable[weaponData->_id] = weaponData;
	}

	// ��
	const rapidjson::Value& vv = doc["armors"];
	for (rapidjson::Value::ConstValueIterator it = vv.Begin(); it != vv.End(); it++) {
		// 
		ArmorData* armorData = new ArmorData();
		
		// id, ��Ī, ��
		armorData->_id = (*it)["id"].GetInt();
		armorData->_name = (*it)["name"].GetString(); 
		armorData->_defence = (*it)["defence"].GetInt();

		// �Ƹ� Ÿ��
		string armorType = (*it)["armorType"].GetString();
		if (armorType.compare("Armor") == 0)
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_ARMOR;
		else if (armorType.compare("Helmet") == 0)
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_HELMET;
		else
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_NONE;
		
		// ������ ���̺� ����
		_itemTable[armorData->_id] = armorData;
	}

	// �Һ��
	const rapidjson::Value& vvv = doc["consumables"];
	for (rapidjson::Value::ConstValueIterator it = vvv.Begin(); it != vvv.End(); it++) {
		//
		ConsumableData* consumableData = new ConsumableData();
		
		// id, ��Ī, ȸ����, �ִ� ���� ����
		consumableData->_id = (*it)["id"].GetInt();
		consumableData->_name = (*it)["name"].GetString();
		consumableData->_recovery = (*it)["recovery"].GetInt();
		consumableData->_maxCount = 100;

		// �Һ��� Ÿ��
		string consumableType = (*it)["consumableType"].GetString();
		if (consumableType.compare("Potion"))
			consumableData->_consumableType = PROTOCOL::ConsumableType::CONSUMABLE_TYPE_POTION;
		else
			consumableData->_consumableType = PROTOCOL::ConsumableType::CONSUMABLE_TYPE_NONE;
		
		// ������ ���̺� ����
		_itemTable[consumableData->_id] = consumableData;
	}

	return true;
}

// ����
bool DataManager::LoadMonsterTable()
{
	rapidjson::Document doc;
	if (!OpenAndParseJson("..\\Common\\Data\\MonsterData.json", doc))
		return false;

	// ����
	const rapidjson::Value& v = doc["monsters"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		MonsterData data;
		
		// id, name
		data.id = (*it)["id"].GetInt();
		data.name = (*it)["name"].GetString();

		// stat
		data.stat.set_level((*it)["stat"]["level"].GetInt());
		data.stat.set_totalexp((*it)["stat"]["totalexp"].GetInt());
		data.stat.set_maxhp((*it)["stat"]["maxhp"].GetInt());
		data.stat.set_hp((*it)["stat"]["hp"].GetInt());
		data.stat.set_damage((*it)["stat"]["damage"].GetInt());
		data.stat.set_defence((*it)["stat"]["defence"].GetInt());
		data.stat.set_speed((*it)["stat"]["speed"].GetFloat());
		data.stat.set_attackdistance((*it)["stat"]["attackdistance"].GetFloat());
		data.stat.set_noticedistance((*it)["stat"]["noticedistance"].GetFloat());
		data.stat.set_returndistance((*it)["stat"]["returndistance"].GetFloat());
		data.stat.set_attackcooltime((*it)["stat"]["attackcooltime"].GetFloat());

		// rewards
		const rapidjson::Value& vv = (*it)["rewards"];
		for (rapidjson::Value::ConstValueIterator it2 = vv.Begin(); it2 != vv.End(); it2++) {
			RewardData rewardData;

			rewardData.itemDropRate = (*it2)["percent"].GetInt();
			rewardData.itemId = (*it2)["itemid"].GetInt();
			rewardData.count = (*it2)["count"].GetInt();

			data.rewardDatas.push_back(rewardData);
		}
		
		_monsterTable[data.id] = data;
	}

	return true;
}

// ��ų
bool DataManager::LoadSkillTable()
{
	// 
	rapidjson::Document doc;
	if (!OpenAndParseJson("..\\Common\\Data\\SkillData.json", doc))
		return false;

	// 
	const rapidjson::Value& v = doc["skills"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {

		//
		SkillData data;

		//
		data.id = (*it)["id"].GetInt();
		data.name = (*it)["name"].GetString();
		data.cooltime = (*it)["cooldown"].GetFloat();
		data.damage = (*it)["damage"].GetInt();
		
		string skillType = (*it)["skillType"].GetString();
		if (skillType.compare("SkillAuto") == 0)
			data.skillType = PROTOCOL::SkillType::SKILL_AUTO;
		else if (skillType.compare("SkillProjectile") == 0)
			data.skillType = PROTOCOL::SkillType::SKILL_PROJECTTILE;
		else
			data.skillType = PROTOCOL::SkillType::SKILL_NONE;

		// 
		_skillTable[data.id] = data;
	}

	return true;
}
