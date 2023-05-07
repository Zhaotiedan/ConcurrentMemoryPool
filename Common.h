#pragma once

#include<iostream>
#include<vector>
#include<thread>
#include<time.h>
#include<Windows.h>
#include<assert.h>
using std::cout;
using std::endl;

static const size_t MAX_BYTES = 256 * 1024;
static const size_t NFREELIST = 208;//最大哈希桶数

class FreeList//管理切割好的小对象空间的自由链表
{
public:
	void Push(void* obj)//头插
	{
		assert(obj);
		*(void**)obj = _head;//取obj的前4/8个字节，也就是obj下一个指向_head
		_head = obj;
	}
	void* Pop()//头删
	{
		assert(_head);
		void* obj = _head;
		_head = *(void**)_head;
		return obj;
	}
	bool Empty()
	{
		return _head == nullptr;
	}
private:
	void* _head = nullptr;
};

class SizeRule//计算对象大小的内存对齐规则
{
public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128]					8byte对齐	    freelist[0,16)
	// [128+1,1024]				16byte对齐	    freelist[16,72)
	// [1024+1,8*1024]			128byte对齐	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte对齐   freelist[184,208)

	/*size_t _RoundUp(size_t size, size_t alignNum)//自我版
	{
		size_t alignSize;//对齐后的大小
		if (size % alignNum != 0)//需要对齐
		{
			alignSize = (size / alignNum + 1) * alignNum;
		}
		else {
			alignSize = size;
		}
	}*/
	//tcmalloc源码版，移位提高效率
	static inline size_t _RoundUp(size_t size, size_t alignNum)//alignNum：对齐数
	{
		return ((size + alignNum - 1) & ~(alignNum - 1));
	}
	//计算内存对齐后的大小
	static inline size_t RoundUp(size_t size)//size：原始需要空间大小
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			assert(false);
			return -1;
		}
	}
	static inline size_t _Index(size_t size, size_t align_shift)//tcmalloc版
	{
		return ((size + (1 << align_shift) - 1) >> align_shift) - 1;
	}
	//计算对齐后的内存大小映射到哪个自由链表哈希桶
	static inline size_t Index(size_t size)
	{
		assert(size <= MAX_BYTES);
		static int group_array[4] = { 16,56,56,56 };//每个区间有多少个桶
		if (size <= 128)
		{
			return _Index(size, 3);
		}
		else if (size <= 1024)
		{
			return _Index(size - 128, 4) + group_array[0];
		}
		else if (size <= 8 * 1024)
		{
			return _Index(size - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (size <= 64 * 1024)
		{
			return _Index(size - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (size <= 256 * 1024)
		{
			return _Index(size - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else {
			assert(false);
		}
		return -1;
	}
};
