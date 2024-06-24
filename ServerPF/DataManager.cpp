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

// 스텟
bool DataManager::LoadStatTable()
{
	// 파일 열고 파싱
	rapidjson::Document doc;
	if (!OpenAndParseJson("..\\Common\\Data\\StatData.json", doc))
		return false;

	// 스텟 데이터 로드
	const rapidjson::Value& v = doc["stats"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		
		//
		StatData statData;

		// 추출
		statData.level = (*it)["level"].GetInt();
		statData.maxhp = (*it)["maxHp"].GetInt();
		statData.damage = (*it)["damage"].GetInt();
		statData.totalExp = (*it)["totalExp"].GetInt();

		// 스탯 테이블에 적재 (레벨로 분류)
		_statTable[statData.level] = statData;
	}

	return true;
}

// 아이템
bool DataManager::LoadItemTable()
{
	// 파일 열고 파싱
	rapidjson::Document doc;
	if (!OpenAndParseJson("..\\Common\\Data\\ItemData.json", doc))
		return false;

	// 무기
	const rapidjson::Value& v = doc["weapons"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		// 
		WeaponData* weaponData = new WeaponData();

		// id, 명칭, 공격력
		weaponData->_id = (*it)["id"].GetInt();
		weaponData->_name = (*it)["name"].GetString();
		weaponData->_damage = (*it)["damage"].GetInt();
		
		// 무기 타입
		string weaponType = (*it)["weaponType"].GetString();
		if (weaponType.compare("Sword") == 0) 
			weaponData->_weaponType = PROTOCOL::WeaponType::WEAPON_TYPE_SWORD;
		else if (weaponType.compare("Axe") == 0) 
			weaponData->_weaponType = PROTOCOL::WeaponType::WEAPON_TYPE_AXE;
		else 
			weaponData->_weaponType = PROTOCOL::WeaponType::WEAPON_TYPE_NONE;
		
		// 아이템 테이블에 적재
		_itemTable[weaponData->_id] = weaponData;
	}

	// 방어구
	const rapidjson::Value& vv = doc["armors"];
	for (rapidjson::Value::ConstValueIterator it = vv.Begin(); it != vv.End(); it++) {
		// 
		ArmorData* armorData = new ArmorData();
		
		// id, 명칭, 방어도
		armorData->_id = (*it)["id"].GetInt();
		armorData->_name = (*it)["name"].GetString(); 
		armorData->_defence = (*it)["defence"].GetInt();

		// 아머 타입
		string armorType = (*it)["armorType"].GetString();
		if (armorType.compare("Armor") == 0)
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_ARMOR;
		else if (armorType.compare("Helmet") == 0)
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_HELMET;
		else
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_NONE;
		
		// 아이템 테이블에 적재
		_itemTable[armorData->_id] = armorData;
	}

	// 소비류
	const rapidjson::Value& vvv = doc["consumables"];
	for (rapidjson::Value::ConstValueIterator it = vvv.Begin(); it != vvv.End(); it++) {
		//
		ConsumableData* consumableData = new ConsumableData();
		
		// id, 명칭, 회복력, 최대 스택 개수
		consumableData->_id = (*it)["id"].GetInt();
		consumableData->_name = (*it)["name"].GetString();
		consumableData->_recovery = (*it)["recovery"].GetInt();
		consumableData->_maxCount = 100;

		// 소비템 타입
		string consumableType = (*it)["consumableType"].GetString();
		if (consumableType.compare("Potion"))
			consumableData->_consumableType = PROTOCOL::ConsumableType::CONSUMABLE_TYPE_POTION;
		else
			consumableData->_consumableType = PROTOCOL::ConsumableType::CONSUMABLE_TYPE_NONE;
		
		// 아이템 테이블에 적재
		_itemTable[consumableData->_id] = consumableData;
	}

	return true;
}

// 몬스터
bool DataManager::LoadMonsterTable()
{
	rapidjson::Document doc;
	if (!OpenAndParseJson("..\\Common\\Data\\MonsterData.json", doc))
		return false;

	// 몬스터
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

// 스킬
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
