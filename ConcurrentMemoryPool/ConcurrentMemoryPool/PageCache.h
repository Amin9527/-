#pragma once

#include"common.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_inst;
	}

	//��ȡһ��span
	Span* NewSpan(size_t npage);

	//��ȡ��ҳ�ŵ�Span��ӳ��
	Span* MapObjectToSpan(void* start);

	//����Span���ϲ�����span
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