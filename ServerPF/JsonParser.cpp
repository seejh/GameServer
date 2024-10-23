#include "pch.h"
#include "JsonParser.h"

#include<fstream>

bool JsonParser::Parse(string path, rapidjson::Document& doc)
{
	// 파일 오픈
	ifstream file(path.c_str());
	if (file.is_open() == false) {
		cerr << "JsonParser::Parse Error - Open File Path: " << path << endl;
		return false;
	}

	// 파일 내용 -> 스트링
	string jsonString((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

	/*string jsonString;
	for (char c; ifs >> c;)
		jsonString += c;*/

	// 제이슨 파싱
	doc.Parse(jsonString.c_str());
	if (doc.HasParseError()) {
		cerr << "JsonParser::Parse Error - Parse File - " << doc.GetParseError() << endl;
		return false;
	}

	return true;
}
