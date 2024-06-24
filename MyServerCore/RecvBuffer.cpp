#include "pch.h"
#include "RecvBuffer.h"


RecvBuffer::RecvBuffer(int size) : _size(size), _readPos(0), _writePos(0)
{
	_capacity = _size * 10;
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer() {


}

bool RecvBuffer::OnRead(int len)
{

	// WritePos - ReadPos

	if (DataSize() < len)
		return false;

	_readPos += len;

	return true;
}

bool RecvBuffer::OnWrite(int len)
{

	// Capacity - WritePos
	if (FreeSize() < len) 
		return false;


	_writePos += len;

	return true;
}

void RecvBuffer::Reset()
{

	const int dataSize = DataSize();

	if (dataSize == 0) {

		_readPos = _writePos = 0;
		return;
	}


	if (FreeSize() < _size) {
		::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
		_writePos = dataSize;
		_readPos = 0;
	}

}

char* RecvBuffer::BufferReadPos()
{
	return &_buffer[_readPos];
}

char* RecvBuffer::BufferWritePos()
{
	return &_buffer[_writePos];
}

int RecvBuffer::DataSize()
{
	return _writePos - _readPos;
}

int RecvBuffer::FreeSize()
{

	return _capacity - _writePos;

}
