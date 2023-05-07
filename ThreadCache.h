#pragma once
#include"Common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);//销毁传大小：为了根据大小映射到哈希桶位置
	// 从中心缓存获取对象
	void* FetchFromCentralCache(size_t index, size_t size);

private:
	FreeList _freelists[NFREELIST];
};

//TLS：线程局部存储,让一个指针指向该thread cache
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;
