#include "pch.h"
#include "SendBuffer.h"

#include"Memory.h"

SendBuffer::SendBuffer(int size) : _size(size)
{
	_buffer = reinterpret_cast<char*>(GMemory->Allocate(size));
}

SendBuffer::~SendBuffer()
{
	if (_buffer)
		GMemory->Release(_buffer);

	_buffer = nullptr;
}
