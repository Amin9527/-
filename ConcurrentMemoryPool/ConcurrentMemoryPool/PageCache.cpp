#include"PageCache.h"

PageCache PageCache::_inst;

//����һ��pagespan�е�span
Span* PageCache::NewSpan(size_t npage)
{
	if (!_pagelist[npage].Empty())  //�����Ӧ��_pagelist����span
	{
		return _pagelist[npage].PopFront();
	}

	for (size_t i = npage + 1; i < NPAGES; ++i)//���û�У�ȥ��߸����span���и�
	{
		SpanList* pagelist = &_pagelist[i];
		if (!pagelist->Empty())
		{
			Span* span = pagelist->PopFront();
			Span* split = new Span();
			split->_pageid = span->_pageid + span->_npage - npage; //�ҵ����г���ҳ��ҳ��
			split->_npage = npage;
			span->_npage -= npage;
			_pagelist[span->_npage].PushFront(span);

			//�������span��split���е�ҳ�ţ���spanӳ��(һ��span�п����ж��ҳ��)
			for (size_t i = 0; i < split->_npage; ++i)
			{
				_id_span_map[split->_pageid + i] = split;
			}
			return split;
		}
	}

	//��Ҫ��ϵͳ�����ڴ�
	void* ptr = VirtualAlloc(NULL, (NPAGES << PAGE_SHIFT), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);  //VirtualAlloc��ҳ�����ڴ�
	if (ptr == nullptr)
	{
		throw std::bad_alloc();
	}

	Span* largespan = new Span();
	largespan->_pageid = (size_t)ptr >> PAGE_SHIFT; //����ַ����12λ����ҳ�ţ�0x00000000=>0,0x00001000=>1,0x00002000=>2(����4kΪһҳ)
	largespan->_npage = NPAGES - 1; //��һ����ϵͳ����128ҳ��С���ڴ�
	_pagelist[NPAGES - 1].PushFront(largespan);

	return NewSpan(npage);
}

//��ȡ��ҳ�ŵ�Span��ӳ��
Span* PageCache::MapObjectToSpan(void* start)
{
	PageID pageid = (size_t)start >> PAGE_SHIFT; //��start��ַת��Ϊ��Ӧ��ҳ��
	auto it = _id_span_map.find(pageid); //map.find()�ҵ����ظ�λ�õĵ��������Ҳ�������end()���ĵ�����
	assert(it != _id_span_map.end());

	return it->second;  //����ӳ���Span*
}

//����span���ϲ�����span
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	//��ǰ�ϲ�
	auto previt = _id_span_map.find(span->_pageid - 1);//��spanǰһҳ��id�Ƿ���map��
	while (previt != _id_span_map.end())
	{
		Span* prevspan = previt->second; //ȡ��ǰһҳpageidӳ���span

		if (prevspan->_usecount != 0) //���ǰһҳӳ���spanû��ȫ���黹,���������ϲ�
			break;

		if ((prevspan->_npage + span->_npage) > (NPAGES - 1))//����ϲ���ҳ�����������ҳ�����������ϲ�
			break;

		_pagelist[prevspan->_npage].Erase(prevspan);
		prevspan->_npage += span->_npage;
		delete span;
		span = prevspan;

		previt = _id_span_map.find(span->_pageid - 1);
	}

	//���ϲ�
	auto nextit = _id_span_map.find(span->_pageid + span->_npage);//��span����span��id�Ƿ���map��
	while (nextit != _id_span_map.end())
	{
		Span* nextspan = nextit->second;//ȡ�������pageidӳ���span

		if (nextspan->_usecount != 0)//�������pageidӳ���spanû��ȫ���黹,���������ϲ�
			break;

		if ((nextspan->_npage + span->_npage) > (NPAGES - 1))//����ϲ���ҳ�����������ҳ�����������ϲ�
			break;

		_pagelist[nextspan->_npage].Erase(nextspan);
		span->_npage += nextspan->_npage;
		delete nextspan;

		nextit = _id_span_map.find(span->_pageid + span->_npage);
	}

	//���ϲ��ɵ���spanӳ�䵽map��
	for (size_t i = 0; i < span->_npage; ++i)
	{
		_id_span_map[span->_pageid + i] = span;
	}
	_pagelist[span->_npage].PushFront(span);//���ϲ��ɵ���span���뵽pagelist��
}