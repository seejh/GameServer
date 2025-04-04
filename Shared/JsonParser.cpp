#include "pch.h"
#include "JsonParser.h"

#include<fstream>

bool JsonParser::ParseFromFile(const char* filePath)
{
    // 파일
    std::ifstream fs(filePath);
    if (fs.is_open() == false) {
        std::cout << "" << std::endl;
        return false;
    }

    // 파일 -> 스트링
    std::string jsonString((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));

    // 스트링 -> Document
    _doc.Parse(jsonString.c_str());
    if (_doc.HasParseError()) {
        std::cerr << "[JsonParser] Parsing Error : " << _doc.GetParseError() << std::endl;
        return false;
    }

    return true;

    /*WString wfilePath = FileUtils::Convert(filePath);
    Vector<BYTE> data = FileUtils::ReadFile(wfilePath.c_str());

    if (data.empty())
        return false;

    _doc.Parse(string(data.begin(), data.end()).c_str());
    if (_doc.HasParseError()) {
        cerr << "[Config] Error : Parsing Error : " << _doc.GetParseError() << endl;
        return false;
    }

    return true;*/
}

rapidjson::Value& JsonParser::operator[](const char* valueName)
{
    return _doc[valueName];
}
