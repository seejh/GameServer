#include "pch.h"
#include "DataManager.h"

#include"DataContents.h"
#include"JsonParser.h"
#include"rapidjson/document.h"

//#include<picojson/picojson.h>
#include<fstream>

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
	if (LoadNpcTable() == false) {
		cout << "LoadNpcTable Error" << endl;
		return false;
	}
	if (LoadQuestTable() == false) {
		cout << "LoadQuestTable Error" << endl;
		return false;
	}

	cout << "[DataManager] Load Data OK" << endl;

	return true;
}

// 스텟
bool DataManager::LoadStatTable()
{
	//
	//picojson::value v;
	//picojson::parse(v, jsonString);


	// Json 파일 파싱
	rapidjson::Document doc;
	if (JsonParser::Parse("..\\Common\\Data\\StatData.json", doc) == false)
		return false;

	// Json 오브젝트(스텟)
	const rapidjson::Value& v = doc["stats"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		
		// StatData 변수에 담고 스텟 테이블에 적재, 레벨로 분류
		StatData statData;

		// 스텟
		statData.level = (*it)["level"].GetInt();
		statData.maxhp = (*it)["maxHp"].GetInt();
		statData.damage = (*it)["damage"].GetInt();
		statData.totalExp = (*it)["totalExp"].GetInt();

		// 테이블 적재
		_statTable[statData.level] = statData;
	}

	return true;
}

// 아이템
bool DataManager::LoadItemTable()
{
	// Json 파일 파싱
	rapidjson::Document doc;
	if (JsonParser::Parse("..\\Common\\Data\\ItemData.json", doc) == false)
		return false;
	
	// Json 오브젝트(무기)
	const rapidjson::Value& v = doc["weapons"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		
		// WeaponData 변수에 담고 무기 테이블에 적재
		WeaponData* weaponData = new WeaponData();
		
		// id, 이름, 공격력
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
		
		// 테이블 적재
		_itemTable[weaponData->_id] = weaponData;
	}

	// Json 오브젝트(방어구)
	const rapidjson::Value& vv = doc["armors"];
	for (rapidjson::Value::ConstValueIterator it = vv.Begin(); it != vv.End(); it++) {
		
		// ArmorData 변수에 담고 방어구 테이블에 적재
		ArmorData* armorData = new ArmorData();
		
		// id, 이름, 방어도
		armorData->_id = (*it)["id"].GetInt();
		armorData->_name = (*it)["name"].GetString(); 
		armorData->_defence = (*it)["defence"].GetInt();

		// 방어구 타입
		string armorType = (*it)["armorType"].GetString();
		if (armorType.compare("Armor") == 0)
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_ARMOR;
		else if (armorType.compare("Helmet") == 0)
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_HELMET;
		else
			armorData->_armorType = PROTOCOL::ArmorType::ARMOR_TYPE_NONE;
		
		// 테이블 적재
		_itemTable[armorData->_id] = armorData;
	}

	// Json 오브젝트(소모품)
	const rapidjson::Value& vvv = doc["consumables"];
	for (rapidjson::Value::ConstValueIterator it = vvv.Begin(); it != vvv.End(); it++) {
		
		// ConsumableData 변수에 담고 소모품 테이블에 적재
		ConsumableData* consumableData = new ConsumableData();
		
		// id, 이름, 회복력, 최대스택개수
		consumableData->_id = (*it)["id"].GetInt();
		consumableData->_name = (*it)["name"].GetString();
		consumableData->_recovery = (*it)["recovery"].GetInt();
		consumableData->_maxCount = 100;

		// 소모품 타입
		string consumableType = (*it)["consumableType"].GetString();
		if (consumableType.compare("HpPotion") == 0)
			consumableData->_consumableType = PROTOCOL::ConsumableType::CONSUMABLE_TYPE_HP_POTION;
		else if (consumableType.compare("MpPotion") == 0)
			consumableData->_consumableType = PROTOCOL::ConsumableType::CONSUMABLE_TYPE_MP_POTION;
		else
			consumableData->_consumableType = PROTOCOL::ConsumableType::CONSUMABLE_TYPE_NONE;
		
		// 테이블 적재
		_itemTable[consumableData->_id] = consumableData;
	}

	return true;
}

// 몬스터
bool DataManager::LoadMonsterTable()
{
	// Json 파일 파싱
	rapidjson::Document doc;
	if (JsonParser::Parse("..\\Common\\Data\\MonsterData.json", doc) == false)
		return false;

	// Json 오브젝트(몬스터)
	const rapidjson::Value& v = doc["monsters"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		// MonsterData 변수에 담고 몬스터 테이블에 적재
		MonsterData* data = new MonsterData();
		
		// id, 이름
		data->id = (*it)["id"].GetInt();
		data->name = (*it)["name"].GetString();

		// 스텟
		data->stat.set_level((*it)["stat"]["level"].GetInt());
		data->stat.set_totalexp((*it)["stat"]["totalexp"].GetInt());
		data->stat.set_maxhp((*it)["stat"]["maxhp"].GetInt());
		data->stat.set_hp((*it)["stat"]["hp"].GetInt());
		data->stat.set_damage((*it)["stat"]["damage"].GetInt());
		data->stat.set_defence((*it)["stat"]["defence"].GetInt());
		data->stat.set_speed((*it)["stat"]["speed"].GetFloat());
		data->stat.set_attackdistance((*it)["stat"]["attackdistance"].GetFloat());
		data->stat.set_noticedistance((*it)["stat"]["noticedistance"].GetFloat());
		data->stat.set_returndistance((*it)["stat"]["returndistance"].GetFloat());
		data->stat.set_attackcooltime((*it)["stat"]["attackcooltime"].GetFloat());

		// 보상
		const rapidjson::Value& vv = (*it)["rewards"];
		for (rapidjson::Value::ConstValueIterator it2 = vv.Begin(); it2 != vv.End(); it2++) {
			RewardData rewardData;

			rewardData.itemDropRate = (*it2)["percent"].GetInt();
			rewardData.itemId = (*it2)["itemid"].GetInt();
			rewardData.count = (*it2)["count"].GetInt();

			data->rewardDatas.push_back(rewardData);
		}
		
		// 테이블 적재
		_monsterTable[data->id] = data;
	}

	return true;
}

// 스킬
bool DataManager::LoadSkillTable()
{
	// Json 파일 파싱
	rapidjson::Document doc;
	if (JsonParser::Parse("..\\Common\\Data\\SkillData.json", doc) == false)
		return false;
	
	// Json 오브젝트(스킬) 
	const rapidjson::Value& v = doc["skills"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {

		// SkillData 변수에 담고 테이블에 적재
		SkillData data;

		// id, 이름, 쿨타임, 스킬공격력
		data.id = (*it)["id"].GetInt();
		data.name = (*it)["name"].GetString();
		data.cooltime = (*it)["cooldown"].GetFloat();
		data.damage = (*it)["damage"].GetInt();
		
		// 스킬 타입
		string skillType = (*it)["skillType"].GetString();
		if (skillType.compare("SkillAuto") == 0)
			data.skillType = PROTOCOL::SkillType::SKILL_AUTO;
		else if (skillType.compare("SkillProjectile") == 0)
			data.skillType = PROTOCOL::SkillType::SKILL_PROJECTTILE;
		else
			data.skillType = PROTOCOL::SkillType::SKILL_NONE;

		// 테이블 적재
		_skillTable[data.id] = data;
	}

	return true;
}

// NPC
bool DataManager::LoadNpcTable()
{
	// 
	rapidjson::Document doc;
	if (JsonParser::Parse("..\\Common\\Data\\NpcData.json", doc) == false)
		return false;

	const rapidjson::Value& v = doc["Npcs"];
	for (rapidjson::Value::ConstValueIterator it = v.Begin(); it != v.End(); it++) {
		NpcData* data = new NpcData();

		// 
		data->id = (*it)["Id"].GetInt();
		data->name = (*it)["Name"].GetString();

		string npcType = (*it)["NpcType"].GetString();
		if (npcType.compare("Normal") == 0)
			data->type = NpcType::NORMAL;
		else if (npcType.compare("Merchant") == 0)
			data->type = NpcType::MERCHANT;
		else
			return false;

		for (const auto& questId : (*it)["Quests"].GetArray()) 
			data->quests.push_back(questId.GetInt());

		// 테이블 적재
		_npcTable[data->id] = data;
	}

	return true;
}

// 퀘스트
bool DataManager::LoadQuestTable()
{
	// 
	rapidjson::Document doc;
	if (JsonParser::Parse("..\\Common\\Data\\QuestData.json", doc) == false)
		return false;

	const rapidjson::Value& v = doc["Quests"];
	for (auto it = v.Begin(); it != v.End(); it++) {
		QuestData* data = new QuestData();

		//
		data->id = (*it)["Id"].GetInt();
		data->name = (*it)["Name"].GetString();

		string type = (*it)["Type"].GetString();
		if (type.compare("Interact") == 0)
			data->type = QuestType::QUEST_TYPE_INTERACT;
		else if (type.compare("Kill") == 0)
			data->type = QuestType::QUEST_TYPE_KILL;
		else if (type.compare("Collect") == 0)
			data->type = QuestType::QUEST_TYPE_COLLECT;
		else
			return false;

		data->objectiveId = (*it)["ObjectiveId"].GetInt();
		data->quantity = (*it)["Quantity"].GetInt();
		data->questGiver = (*it)["QuestGiver"].GetInt();
		data->rewardExp = (*it)["RewardExp"].GetInt();
		data->rewardMoney = (*it)["RewardMoney"].GetInt();
		
		for (const auto& Item : (*it)["RewardItems"].GetArray()) 
			data->rewardItems[Item["ItemId"].GetInt()] = Item["ItemQuantity"].GetInt();

		// 테이블 적재
		_questTable[data->id] = data;
	}

	return true;
}
