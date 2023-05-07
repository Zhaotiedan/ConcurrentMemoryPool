#pragma once
#include"Common.h"

inline static void* SystemAlloc(size_t kpage)//һҳ��СΪ8Kb 
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage * 8 * 1024, MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);
#else
	// linux��brk mmap��
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}
template<class T>
class MemoryPool
{
public:
	T* New()//�����µĿռ䣬����ָ�����ռ��ָ��
	{
		T* obj = nullptr;//ָ���¿��ٵ����ռ�
		//�ȼ�����������Ƿ��п��пռ䣬�еĻ���������
		if (_freeList)
		{
			void* next = *(void**)_freeList;//ȡǰ4/8���ֽڣ�Ҳ������һ���ڴ�
			obj = (T*)_freeList;
			_freeList = next;
		}
		else
		{
			if (_remainBytes < sizeof(T))//���ʣ���ֽ�����������T���Ͷ������¿��ٿռ�
			{
				_remainBytes = 128 * 1024;
				//_memory = (char*)malloc(_remainBytes);
				_memory = (char*)SystemAlloc(_remainBytes>>13);//����ȥ����Ϊҳ��
				if (_memory == nullptr)//����ʧ��
				{
					throw std::bad_alloc();
				}
			}
			obj = (T*)_memory;//��ǰ����ռ��ַΪ_memoryָ���ַ
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);//����洢Ԫ�ش�СС��ָ�룬������Ҫ����һ��ָ���С�Ŀռ�
			_memory += sizeof(T);//����_memory
			_remainBytes -= sizeof(T);//����_remainBytes
		}
		//��ʾ����T�Ĺ��캯����ʼ��
		new(obj)T;
		return obj;
	}
	void Delete(T* obj)
	{
		obj->~T();
		//ͷ�嵽freeList
		*(void**)obj = _freeList;
		//void**:ָ��һ��ָ�룬����32��64λ��*(void**)��С����ָ���С,����˳����ȡ�ÿռ��ǰ4/8���ֽ�
		_freeList = obj;
	}
private:
	char* _memory = nullptr;//ָ��ÿ������Ĵ���ڴ��ָ��
	size_t _remainBytes = 0;//����ڴ��зֹ����е�ʣ���ֽ���
	void* _freeList = nullptr;//ָ�����ӻ����ڴ����������ͷָ��
};

struct TreeNode
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;
	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};
void TestObjectPool()
{
	
	const size_t Rounds = 3;// �����ͷŵ��ִ�
	const size_t N = 100000;// ÿ�������ͷŶ��ٴ�
	size_t begin1 = clock();
	std::vector<TreeNode*> v1;
	v1.reserve(N);
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}
	size_t end1 = clock();
	std::vector<TreeNode*> v2;
	v2.reserve(N);

	MemoryPool<TreeNode> TNPool;
	size_t begin2 = clock();
	
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
		v2.push_back(TNPool.New());
		}
		for (int i = 0; i < N; ++i)
		{
		TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();
	cout << "new cost time:" << end1 - begin1 << endl;
	cout << "object pool cost time:" << end2 - begin2 << endl;
}