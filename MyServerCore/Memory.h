#pragma once

#include"Allocator.h"


/*---------------------------------------------------------------
	Memory
------------------------------------------------------------------*/

class MemoryPool;

class Memory {
	enum {
		// ~1024(32����, 32��), 1025~2048(128����, 8��), 2049~4096(256����, 8��) = 48��
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096,
	};

public:
	Memory();
	~Memory();

	void* Allocate(int size);
	void Release(void* ptr);
private:
	// �޸�Ǯ ���հ�, ���� ã�� ���� ���̺� O(1)
	vector<MemoryPool*> _pools;
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};





/*------------------------------------------------------------------
	�޸� �Ҵ� Ŀ����(�޸�Ǯ ���) ������
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