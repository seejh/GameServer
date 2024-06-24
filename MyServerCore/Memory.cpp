#include "pch.h"
#include "Memory.h"

#include"MemoryPool.h"


Memory::Memory() {
	int size = 0;
	int tableIndex = 0;

	// ~ 1024, 32단위, 메모리풀 32개

	while (size < 1024) {
		size += 32;


		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size) {
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	// 1025 ~ 2048, 128단위, 메모리풀 8개

	while (size < 2048) {
		size += 128;


		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size) {
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	// 2049 ~ 4096, 256단위, 메모리풀 8개

	while (size < 4096) {
		size += 256;


		MemoryPool* pool = new MemoryPool(size);
		_pools.push_back(pool);

		while (tableIndex <= size) {
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}
}

Memory::~Memory() {
	for (MemoryPool* pool : _pools)
		delete pool;

	_pools.clear();
}

void* Memory::Allocate(int size) {
	MemoryHeader* header;
	const int allocSize = size + sizeof(MemoryHeader);

	if (allocSize > MAX_ALLOC_SIZE)
		header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	else
		header = _poolTable[allocSize]->Pop();

	return MemoryHeader::AttachHeader(header, allocSize);
}

void Memory::Release(void* ptr) {
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int allocSize = header->allocSize;
	ASSERT_CRASH(allocSize > 0);

	if (allocSize > MAX_ALLOC_SIZE)
		::_aligned_free(header);
	else
		_poolTable[allocSize]->Push(header);
}

