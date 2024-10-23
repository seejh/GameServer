#pragma once

#include"rapidjson/document.h"

//class JsonObject {
//
//};

class JsonParser
{
public:
	static bool Parse(string path, rapidjson::Document& doc);
	
public:
	rapidjson::Document doc;
};

