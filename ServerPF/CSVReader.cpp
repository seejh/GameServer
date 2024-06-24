#include "pch.h"
#include "CSVReader.h"


bool CSVReader::OpenFile(string& filePath)
{
	// reset
	input.close();
	input.clear();

	// open new file
	input.open(filePath.c_str());

	// is open?
	if (!input.is_open()) {
		cout << "CSVReader Error : can't open file, wrong file path." << endl;
		return false;
	}

	return true;
}

CSVReader* CSVReader::Instance()
{
	static CSVReader instance;
	return &instance;
}

bool CSVReader::Read(string filePath, OUT vector<vector<string>>& csvRows)
{
	if (OpenFile(filePath)) {
		// 한 행을 가져옴
		for (string line; getline(input, line);) {
			istringstream iss(move(line));
			vector<string> row;

			if (!csvRows.empty())
				row.reserve(csvRows.front().size());

			// ,를 기준으로 분리
			for (string value; getline(iss, value, ',');)
				row.push_back(move(value));

			csvRows.push_back(move(row));
		}

		return true;
	}

	return false;
}
