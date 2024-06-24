#pragma once

enum { SLIST_ALIGNMENT = 16, };

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY {
	MemoryHeader(int size) : allocSize(size) {}

	static void* AttachHeader(MemoryHeader* header, int size) {
		new(header)MemoryHeader(size);
		return reinterpret_cast<void*>(++header);
	}

	static MemoryHeader* DetachHeader(void* ptr) {
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		return header;
	}

	int allocSize;
};

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool {
public:
	MemoryPool(int allocSize);
	~MemoryPool();

	void Push(MemoryHeader* ptr);
	MemoryHeader* Pop();
private:
	SLIST_HEADER _header;
	int _allocSize = 0;
	atomic<int> _useCount = 0;
	atomic<int> _reserveCount = 0;
};