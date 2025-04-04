#include"pch.h"
#include"ConfigManager.h"

//#include<fstream>
//#include<string>

/*------------------------------------------------------------------------------------
	Config
-------------------------------------------------------------------------------------*/
std::string Config::WstrToStr(std::wstring wstr)
{
	const int wstrLen = static_cast<int>(wstr.size());
	if (wstrLen == 0)
		return std::string();

	int needSize = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstrLen, NULL, 0, NULL, NULL);
	std::string str(needSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstrLen, &str[0], needSize, NULL, NULL);

	return str;
}

std::wstring Config::StrToWstr(std::string str)
{
	const int strLen = static_cast<int>(str.size());
	if (strLen == 0)
		return std::wstring();

	int needSize = MultiByteToWideChar(CP_UTF8, 0, &str[0], strLen, NULL, 0);
	std::wstring wstr(needSize, 0);
	::MultiByteToWideChar(CP_UTF8, 0, &str[0], strLen, &wstr[0], needSize);

	return wstr;
}

/*------------------------------------------------------------------------------------
	ConfigManager
-------------------------------------------------------------------------------------*/

bool ConfigManager::LoadConfigFile(const char* filePath)
{
	if (_parser.ParseFromFile(filePath) == false) {
		std::cout << "[ConfigManager] Error : Config File Load Failed" << std::endl;
		return false;
	}

	std::cout << "[ConfigManager] Config File Load OK" << std::endl;

	return true;
}

bool ConfigManager::LoadConfigByName(const char* configName, Config* config)
{
	if (config == nullptr || _parser[configName].IsNull() == true)
		return false;

	switch (config->_type) {
	case ConfigType::SERVER:
	{
		ServerConfig* serverConfig = reinterpret_cast<ServerConfig*>(config);
		serverConfig->_serverDbId = _parser[configName]["ServerDbId"].GetInt();
		serverConfig->_serverName = _parser[configName]["ServerName"].GetString();
		serverConfig->_ip = _parser[configName]["Ip"].GetString();
		serverConfig->_port = _parser[configName]["Port"].GetInt();
		serverConfig->_threadCount = _parser[configName]["ThreadCount"].GetInt();
	}
	break;

	case ConfigType::RDB:
	{
		RDBConfig* rdbConfig = reinterpret_cast<RDBConfig*>(config);
		rdbConfig->_connectionString = _parser[configName]["ConnectionString"].GetString();
		rdbConfig->_connectionCount = _parser[configName]["ConnectionCount"].GetInt();
	}
	break;

	case ConfigType::MDB:
	{
		MDBConfig* mdbConfig = reinterpret_cast<MDBConfig*>(config);
		mdbConfig->_ip = _parser[configName]["Ip"].GetString();
		mdbConfig->_port = _parser[configName]["Port"].GetInt();
		mdbConfig->_connectionCount = _parser[configName]["ConnectionCount"].GetInt();
	}
	break;

	default:
		break;
	}
	return true;
}

