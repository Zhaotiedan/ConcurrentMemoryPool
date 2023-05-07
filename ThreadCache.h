#pragma once
#include"Common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);//���ٴ���С��Ϊ�˸��ݴ�Сӳ�䵽��ϣͰλ��
	// �����Ļ����ȡ����
	void* FetchFromCentralCache(size_t index, size_t size);

private:
	FreeList _freelists[NFREELIST];
};

//TLS���ֲ߳̾��洢,��һ��ָ��ָ���thread cache
static _declspec(thread) ThreadCache* pTLSThreadCache = nullptr;
