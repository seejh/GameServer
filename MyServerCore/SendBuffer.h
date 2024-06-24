#pragma once

// 가변적인 공간에 대해 풀로써 할당 받고, 사용이 끝나면 알아서 반납되게
// 다만 send가 실패할 경우, 이 메모리의 처리에 대해서 고민
// shared_ptr로 관리되어야 한다.
class SendBuffer
{
public:
	SendBuffer(int size);
	~SendBuffer();

	char* Buffer() { return _buffer; }
	int Size() { return _size; }
public:
	char* _buffer;
	int _size;
};

