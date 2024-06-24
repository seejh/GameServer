#pragma once

class RecvBuffer
{
public:

	RecvBuffer(int size = 1000);

	~RecvBuffer();

	bool OnRead(int len);
	bool OnWrite(int len);
	void Reset();

	char* BufferReadPos();
	char* BufferWritePos();

	int DataSize();
	int FreeSize();
public:

	vector<char, StlAllocator<char>> _buffer;
	int _capacity;

	int _size;
	int _readPos;
	int _writePos;
};

