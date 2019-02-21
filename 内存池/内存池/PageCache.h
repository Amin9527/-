#pragma once

#include"common.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_inst;
	}

	//获取一个span
	Span* NewSpan(size_t npage);

	//获取从页号到Span的映射
	Span* MapObjectToSpan(void* start);

	//回收Span，合并相邻span
	void ReleaseSpanToPageCache(Span* span);
private:
	SpanList _pagelist[NPAGES];
	std::map<PageID, Span*> _id_span_map;
private:
	PageCache() = default;
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;

	static PageCache _inst;
};