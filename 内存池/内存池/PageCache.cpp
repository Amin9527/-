#include"PageCache.h"

PageCache PageCache::_inst;

//申请一个pagespan中的span
Span* PageCache::NewSpan(size_t npage)
{
	if (!_pagelist[npage].Empty())
	{
		return _pagelist[npage].PopFront();
	}

	for (size_t i = npage + 1; i < NPAGES; ++i)
	{
		SpanList* pagelist = &_pagelist[i];
		if (!pagelist->Empty())
		{
			Span* span = pagelist->PopFront();
			Span* split = new Span();
			split->_pageid = span->_pageid + span->_npage - npage;
			split->_npage = npage;
			span->_npage -= npage;
			_pagelist[span->_npage].PushFront(span);

			//将申请的span（split）中的页号，与split映射
			for (size_t i = 0; i < split->_npage; ++i)
			{
				_id_span_map[split->_pageid + i] = split;
			}
			return split;
		}
	}

	//需要向系统申请内存
	void* ptr = VirtualAlloc(NULL, (NPAGES << PAGE_SHIFT), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (ptr == nullptr)
	{
		throw std::bad_alloc();
	}

	Span* largespan = new Span();
	largespan->_pageid = (size_t)ptr >> PAGE_SHIFT;
	largespan->_npage = NPAGES - 1;
	_pagelist[NPAGES - 1].PushFront(largespan);

	return NewSpan(npage);
}

//获取从页号到Span的映射
Span* PageCache::MapObjectToSpan(void* start)
{
	PageID pageid = (size_t)start >> PAGE_SHIFT; //将start地址转换为相应的页号
	auto it = _id_span_map.find(pageid); //map.find()找到返回该位置的迭代器，找不到返回end()处的迭代器
	assert(it != _id_span_map.end());

	return it->second;  //返回映射的Span*
}

//回收span，合并相邻span
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	auto previt = _id_span_map.find(span->_pageid - 1);//找span前一页的id是否在map中
	while (previt != _id_span_map.end())
	{
		Span* prevspan = previt->second; //取出前一页pageid映射的span

		if (prevspan->_usecount == 0) //如果前一页映射的span没有全部归还,跳出，不合并
			break;

		if ((prevspan->_npage + span->_npage) > (NPAGES - 1))//如果合并的页数超过了最大页，跳出，不合并
			break;

		_pagelist[prevspan->_npage].Erase(prevspan);
		prevspan->_npage += span->_npage;
		delete span;
		span = prevspan;

		previt = _id_span_map.find(span->_pageid - 1);
	}

	auto nextit = _id_span_map.find(span->_pageid + span->_npage);
	while (nextit != _id_span_map.end())
	{
		Span* nextspan = nextit->second;

		if (nextspan->_usecount != 0)
			break;

		if ((nextspan->_npage + span->_npage) > (NPAGES - 1))
			break;

		_pagelist[nextspan->_npage].Erase(nextspan);
		span->_npage += nextspan->_npage;
		delete nextspan;

		nextit = _id_span_map.find(span->_pageid + span->_npage);
	}

	for (size_t i = 0; i < span->_npage; ++i)
	{
		_id_span_map[span->_pageid + i] = span;
	}
	_pagelist[span->_npage].PushFront(span);
}