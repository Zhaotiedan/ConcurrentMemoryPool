#pragma once
#include"Common.h"

inline static void* SystemAlloc(size_t kpage)//一页大小为8Kb 
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage * 8 * 1024, MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);
#else
	// linux下brk mmap等
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}
template<class T>
class MemoryPool
{
public:
	T* New()//申请新的空间，返回指向这块空间的指针
	{
		T* obj = nullptr;//指向新开辟的这块空间
		//先检查自由链表是否有空闲空间，有的话优先申请
		if (_freeList)
		{
			void* next = *(void**)_freeList;//取前4/8个字节，也就是下一个内存
			obj = (T*)_freeList;
			_freeList = next;
		}
		else
		{
			if (_remainBytes < sizeof(T))//如果剩余字节数不够申请T类型对象，重新开辟空间
			{
				_remainBytes = 128 * 1024;
				//_memory = (char*)malloc(_remainBytes);
				_memory = (char*)SystemAlloc(_remainBytes>>13);//传进去参数为页数
				if (_memory == nullptr)//申请失败
				{
					throw std::bad_alloc();
				}
			}
			obj = (T*)_memory;//当前分配空间地址为_memory指向地址
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);//如果存储元素大小小于指针，则至少要开辟一个指针大小的空间
			_memory += sizeof(T);//更新_memory
			_remainBytes -= sizeof(T);//更新_remainBytes
		}
		//显示调用T的构造函数初始化
		new(obj)T;
		return obj;
	}
	void Delete(T* obj)
	{
		obj->~T();
		//头插到freeList
		*(void**)obj = _freeList;
		//void**:指向一个指针，不管32或64位，*(void**)大小都是指针大小,可以顺利获取该空间的前4/8个字节
		_freeList = obj;
	}
private:
	char* _memory = nullptr;//指向每次申请的大块内存的指针
	size_t _remainBytes = 0;//大块内存切分过程中的剩余字节数
	void* _freeList = nullptr;//指向链接还回内存自由链表的头指针
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
	
	const size_t Rounds = 3;// 申请释放的轮次
	const size_t N = 100000;// 每轮申请释放多少次
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