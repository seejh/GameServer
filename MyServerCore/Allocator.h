#pragma once


class PoolAllocator {
public:
	static void* Allocate(int size);
	static void Release(void* ptr);
};

template<typename T>
class StlAllocator {
public:
	using value_type = T;

	StlAllocator() {}

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) {}

	T* allocate(size_t count) {
		const int size = static_cast<int>(count * sizeof(T));
		return static_cast<T*>(PoolAllocator::Allocate(size));
	}

	void deallocate(T* ptr, size_t count) {
		PoolAllocator::Release(ptr);
	}

	template<typename U>
	bool operator==(const StlAllocator<U>&) { return true; }

	template<typename U>
	bool operator!=(const StlAllocator<U>&) { return false; }
};
