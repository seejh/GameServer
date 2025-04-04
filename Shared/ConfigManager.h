#pragma once

#include"JsonParser.h"

enum class ConfigType { SERVER, RDB, MDB };

/*--------------------------------------------------------------------
	Config
--------------------------------------------------------------------*/

class Config {
public:
	Config(ConfigType type) : _type(type) {};

	static std::string WstrToStr(std::wstring wstr);
	static std::wstring StrToWstr(std::string str);

public:
	ConfigType _type;
};

class ServerConfig : public Config {
public:
	ServerConfig() : Config(ConfigType::SERVER) {}
public:
	int _serverDbId;
	std::string _serverName;
	std::string _ip;
	int _port;
	int _threadCount;
};
class RDBConfig : public Config {
public:
	RDBConfig() : Config(ConfigType::RDB) {}
public:
	std::string _connectionString;
	int _connectionCount;
};
class MDBConfig : public Config {
public:
	MDBConfig() : Config(ConfigType::MDB) {}
public:
	std::string _ip;
	int _port;
	int _connectionCount;
};

/*--------------------------------------------------------------------
	ConfigManager
--------------------------------------------------------------------*/
class ConfigManager {
public:
	static ConfigManager* Instance() {
		static ConfigManager instance;
		return &instance;
	}

	bool LoadConfigFile(const char* filePath);

	bool LoadConfigByName(const char* configName, Config* config);

private:
	ConfigManager() {}
	ConfigManager(const ConfigManager&) {}
	ConfigManager& operator=(const ConfigManager&) {}

public:
	JsonParser _parser;
};