#pragma once

class ServerConfig {
public:
	int32 ServerDbId;
	wstring Name;
	wstring IpAddress;
	int32 Port;
};

class ConfigManager {
public:
	static ConfigManager* Instance() {
		static ConfigManager instance;
		return &instance;
	}

	bool Init();
	bool LoadConfig();

public:
	ServerConfig _config;
};