#pragma once

#include<fstream>
#include<sstream>

class CSVReader
{
private:
	CSVReader() {}
	CSVReader(const CSVReader&);
	CSVReader& operator=(const CSVReader&);

	bool OpenFile(string& filePath);
public:
	static CSVReader* Instance();

	bool Read(string filePath, OUT vector<vector<string>>& csvRows);
private:
	ifstream input;
};

