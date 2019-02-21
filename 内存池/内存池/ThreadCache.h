#pragma once

#include"common.h"

class ThreadCache
{
public:
	//获取一块size大小的内存
	void* Allocate(size_t size);

	//到中心块取内存
	void* FetchFromCentralCache(size_t index, size_t size);

	//释放申请的内存
	void Deallocate(void* ptr , size_t size);

	//自由链表中挂载的对象太多，开始回收
	void ListTooLong(FreeList* freelist,size_t bytes);
private:
	FreeList _freelist[NLISTS];
};