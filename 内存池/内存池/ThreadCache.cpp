#include"ThreadCache.h"
#include"CentralCache.h"

//到中心块取内存
void* ThreadCache::FetchFromCentralCache(size_t index, size_t bytes)
{
	FreeList* freelist = &_freelist[index];
	size_t num_to_move = min(ClassSize::NumMoveSize(bytes), freelist->MaxSize());//num_to_move是想要申请对象的个数
	void *start, *end;
	size_t fetchnum = CentralCache::GetInstance()->FetchRangeObj(start, end, num_to_move, bytes);//fetch是实际申请的对象个数
	if (fetchnum > 1)
	{
		freelist->PushRange(NEXT_OBJ(start), end, fetchnum - 1);
	}

	return start;
}

//开辟size字节大小的内存
void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAXBYTES);

	//对齐取整
	size = ClassSize::Roundup(size);
	//计算取整后内存块对应freelist链表中的位置
	size_t index = ClassSize::Index(size);
	FreeList* freelist = &_freelist[index];
	if (!freelist->Empty())
	{
		return freelist->Pop();//取出相应位置内存块并返回
	}
	else
	{
		return FetchFromCentralCache(index, size);//到中心块取内存
	}
}

//自由链表下挂载的对象太多，开始回收
void ThreadCache::ListTooLong(FreeList* freelist, size_t bytes)
{
	void* start = freelist->Clear();
	CentralCache::GetInstance()->ReleaseListToSpan(start, bytes);
}