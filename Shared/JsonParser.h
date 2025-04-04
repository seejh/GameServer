#pragma once


#include"../Library/include/rapidjson/document.h"
#include"../Library/include/rapidjson/writer.h"
#include"../Library/include/rapidjson/stringbuffer.h"


class JsonParser
{
public:
	bool ParseFromFile(const char* filePath);

	rapidjson::Value& operator[](const char* valueName);
public:
	rapidjson::Document _doc;
};

