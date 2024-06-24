#include "pch.h"
#include "Allocator.h"

#include"Memory.h"

void* PoolAllocator::Allocate(int size)
{
	return GMemory->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemory->Release(ptr);
}
