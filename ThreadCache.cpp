#include"ThreadCache.h"


void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	return  nullptr;
}
void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAX_BYTES);
	size_t alignSize = SizeRule::RoundUp(size);//�������ڴ��С
	size_t index = SizeRule::Index(size);//��Ӧ��ϣͰ���±�

	if (!_freelists[index].Empty())//��Ӧ���ɹ�ϣͰ��Ϊ�գ�����ռ�
	{
		return _freelists[index].Pop();
	}
	else//��Ӧ��ϣͰΪ�գ���central cache��ȡ�ڴ�
	{
		return FetchFromCentralCache(index, alignSize);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size)
{
	assert(ptr);
	assert(size <= MAX_BYTES);
	//�����ڴ棬�ҵ���Ӧ���ɹ�ϣͰ
	size_t index = SizeRule::Index(size);//�����Ӧ��ϣͰ�±�
	_freelists[index].Push(ptr);
}

