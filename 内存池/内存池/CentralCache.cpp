#include"CentralCache.h"
#include"PageCache.h"

CentralCache CentralCache::_inst;

//获取一个span
Span* CentralCache::GetOneSpan(SpanList* spanlist, size_t bytes)
{
	Span* span = spanlist->begin();//获取带头双向循环链表头结点的下一个节点
	while (span != spanlist->end())//循环找一个有对象的span
	{
		if (span->_objlist != nullptr)
			return span;
		span = span->_next;
	}

	//在CentralCache中没有找到span，向pagecache申请一个新的合适大小的span

	size_t npage = ClassSize::NumMovePage(bytes);
	Span* newspan = PageCache::GetInstance()->NewSpan(npage);

	//将newspan的内存切割成一个个bytes大小的对象挂起来
	char* start = (char*)(newspan->_pageid << PAGE_SHIFT);
	char* end = start + (newspan->_npage << PAGE_SHIFT);
	char* cur = start;
	char* next = start+bytes;
	while (next < end)
	{
		NEXT_OBJ(cur) = next;
		cur = next;
		next += bytes;
	}
	NEXT_OBJ(cur) = nullptr;
	newspan->_objlist = start;
	newspan->_objsize = bytes;
	newspan->_usecount = 0;

	//将newspan插入到SpanList中
	spanlist->PushFront(newspan);
	return newspan;
}

//从中心获取一定数量的对象给threadcache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t num, size_t bytes)
{
	size_t index = ClassSize::Index(bytes);//找到该大小内存对应自由链表的位置标识
	SpanList* spanlist = &_spanlist[index];//找到该大小内存对应自由链表的位置

	//获取一个span
	Span* span = GetOneSpan(spanlist, bytes);
	void* cur = span->_objlist;
	void* prev = cur;
	size_t fetchnum = 0;
	while (cur != nullptr && fetchnum < num)//在对应的span悬挂对象链表中取num个对象，
	{                                       //如果不够num个对象，则全部取出
		prev = cur;
		cur = NEXT_OBJ(cur);
		++fetchnum;
	}

	NEXT_OBJ(prev) = nullptr;//将取出的最后一个对象的next制空
	start = span->_objlist;
	end = prev;
	span->_objlist = cur;//span的_objlist指向剩余对象
	span->_usecount += fetchnum;//使用计数增加
	
	return fetchnum;
}

//将一定数量的对象释放回span
void CentralCache::ReleaseListToSpan(void* start, size_t bytes)
{
	size_t index = ClassSize::Index(bytes);
	SpanList* spanlist = &_spanlist[index];

	while (start)
	{
		void* next = NEXT_OBJ(start);
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		NEXT_OBJ(start) = span->_objlist;
		NEXT_OBJ(span->_objlist) = start;

		//usecount==0 表示span切出的对象全部收回
		//释放span回到PageCache进行合并
		if (--span->_usecount == 0)
		{
			spanlist->Erase(span); //将span从spanlist链表去掉

			span->_next == nullptr;
			span->_prev == nullptr;
			span->_objsize == 0;
			span->_objlist == nullptr;

			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		}
		start = next;
	}
}