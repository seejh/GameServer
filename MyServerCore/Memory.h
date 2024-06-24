#pragma once

#include"Allocator.h"


/*---------------------------------------------------------------
	Memory
------------------------------------------------------------------*/

class MemoryPool;

class Memory {
	enum {
		// ~1024(32단위, 32개), 1025~2048(128단위, 8개), 2049~4096(256단위, 8개) = 48개
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096,
	};

public:
	Memory();
	~Memory();

	void* Allocate(int size);
	void Release(void* ptr);
private:
	// 메모리풀 집합과, 빨리 찾기 위한 테이블 O(1)
	vector<MemoryPool*> _pools;
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};





/*------------------------------------------------------------------
	메모리 할당 커스텀(메모리풀 사용) 연산자
	xnew, xdelete, MakeShared
-------------------------------------------------------------------*/

template<typename Type, typename... Args>
Type* xnew(Args&&... args) {
	Type* memory = static_cast<Type*>(PoolAllocator::Allocate(sizeof(Type)));

	// placement new
	new(memory)Type(std::forward<Args>(args)...);
	
	return memory;
}



template<typename Type>
void xdelete(Type* obj) {
	obj->~Type();

	PoolAllocator::Release(obj);

}

template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args) {
	return shared_ptr<Type>{ xnew<Type>(forward<Args>(args)...), xdelete<Type> };

}