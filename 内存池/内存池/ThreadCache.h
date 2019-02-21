#pragma once

#include"common.h"

class ThreadCache
{
public:
	//��ȡһ��size��С���ڴ�
	void* Allocate(size_t size);

	//�����Ŀ�ȡ�ڴ�
	void* FetchFromCentralCache(size_t index, size_t size);

	//�ͷ�������ڴ�
	void Deallocate(void* ptr , size_t size);

	//���������й��صĶ���̫�࣬��ʼ����
	void ListTooLong(FreeList* freelist,size_t bytes);
private:
	FreeList _freelist[NLISTS];
};