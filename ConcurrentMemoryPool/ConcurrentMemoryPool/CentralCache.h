#pragma once
#include"common.h"

//单例模式的CentralCache
class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &_inst;
	}

	//获取一个span
	Span* GetOneSpan(SpanList* spanlist, size_t bytes);

	//从中心获取一定数量的对象给threadcache
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t bytes);

	//将一定数量的对象释放回span
	void ReleaseListToSpan(void* start, size_t bytes);
private:
	//中心缓存自由链表
	SpanList _spanlist[NLISTS];
private:
	CentralCache() = default;  //使用默认的构造函数
	CentralCache(const CentralCache&) = delete;
	CentralCache& operator=(const CentralCache&) = delete;

	static CentralCache _inst;
};