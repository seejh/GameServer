#include"pch.h"
#include "ConfigManager.h"

#include"JsonParser.h"
#include"rapidjson/document.h"

bool ConfigManager::Init()
{
	if (LoadConfig() == false) {
		cerr << "ConfigManager Init Error : Load File" << endl;
		return false;
	}

	cout << "[ConfigManager] Init OK" << endl;

	return true;
}

bool ConfigManager::LoadConfig()
{
	// Json ÆÄÀÏ ÆÄ½Ì
	rapidjson::Document doc;
	if (JsonParser::Parse("..\\Common\\Data\\ServerConfig.json", doc) == false)
		return false;

	// 
	const rapidjson::Value& ServerDbId = doc["ServerDbId"];
	const rapidjson::Value& Name = doc["Name"];
	const rapidjson::Value& Ip = doc["IpAddress"];
	const rapidjson::Value& Port = doc["Port"];

	_config.ServerDbId = ServerDbId.GetInt();
	// string  -> wstring
	{
		string name = Name.GetString();
		int needSize = MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), NULL, 0);
		wstring wname(needSize, 0);
		MultiByteToWideChar(CP_UTF8, 0, &name[0], (int)name.size(), &wname[0], needSize);
		_config.Name = wname;
	} 
	{
		string ip = Ip.GetString();
		int needSize = MultiByteToWideChar(CP_UTF8, 0, &ip[0], (int)ip.size(), NULL, 0);
		wstring wip(needSize, 0);
		MultiByteToWideChar(CP_UTF8, 0, &ip[0], (int)ip.size(), &wip[0], needSize);
		_config.IpAddress = wip;
	}
	_config.Port = Port.GetInt();

	return true;
}
