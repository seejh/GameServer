#pragma once

/// MACRO
#include"Allocator.h"
#include<map>
#include<set>
#include<vector>

#define OUT

#define NAMESPACE_BEGIN(name)	namespace name {
#define NAMESPACE_END			}

#define USING_SHARED_PTR(name) using name##Ref = std::shared_ptr<class name>;

#define size32(val) static_cast<int32>(sizeof(val))

/*-----------------------------------------------------------------------
	Assert
------------------------------------------------------------------------*/
#define CRASH(cause){					\
	unsigned int* crash = nullptr;		\
	__analysis_assume(crash != nullptr);\
	*crash = 0xDEADBEEF;				\
}

#define ASSERT_CRASH(expr){		\
	if (!(expr)) {				\
		CRASH("ASSERT_CRASH");	\
		__analysis_assume(expr);\
	}							\
}


/*-----------------------------------------------------------------------
	Lock
-----------------------------------------------------------------------*/
#define USE_MANY_LOCKS(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCKS(1)
#define READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0)


/*------------------------------------------------------------------------
	
-------------------------------------------------------------------------*/
using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;

using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

template<typename T>
using Vector = vector<T, StlAllocator<T>>;

template<typename Key, typename Pred = less<Key>>
using Set = set<Key, Pred, StlAllocator<Key>>;

template<typename Key, typename Type, typename Pred = less<Key>>
using Map = map<Key, Type, Pred, StlAllocator<pair<const Key, Type>>>;

using WString = std::basic_string<wchar_t, char_traits<wchar_t>, StlAllocator<wchar_t>>;

/*
	1. wstring -> string
	wstring wstr = ;
	int needSize = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	string str(needSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], needSize, NULL, NULL);
	
	2. string -> wstring
	string str = ;
	int needSize = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	wstring wstr(needSize, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], needSize);
*/