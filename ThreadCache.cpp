#include"ThreadCache.h"


void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	return  nullptr;
}
void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	size_t alignSize = SizeRule::RoundUp(size);//对齐后的内存大小
	size_t index = SizeRule::Index(size);//对应哈希桶的下标

	if (!_freelists[index].Empty())//对应自由哈希桶不为空，分配空间
	{
		return _freelists[index].Pop();
	}
	else//对应哈希桶为空，向central cache获取内存
	{
		return FetchFromCentralCache(index, alignSize);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);
	//回收内存，挂到对应自由哈希桶
	size_t index = SizeRule::Index(size);//计算对应哈希桶下标
	_freelists[index].Push(ptr);
}

