#pragma once
#include "MemoryPool.h"

template<typename T>
class ObjectPool
{
public:
	template<typename... Args>
	static T* Pop(Args&&... args) {
		T* memory = static_cast<T*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));

		new(memory)T(forward<Args>(args)...);
		return memory;
	}

	static void Push(T* obj) {
		obj->~T();

		s_pool.Push(MemoryHeader::DetachHeader(obj));
	}

	template<typename... Args>
	static shared_ptr<T> MakeShared(Args&&... args) {
		shared_ptr<T> ptr = { Pop(forward<Args>(args)...), Push };
		return ptr;
	}

private:
	static int32 s_allocSize;
	static MemoryPool s_pool;
};

template<typename T>
int32 ObjectPool<T>::s_allocSize = sizeof(T) + sizeof(MemoryHeader);

template<typename T>
MemoryPool ObjectPool<T>::s_pool{ s_allocSize };

