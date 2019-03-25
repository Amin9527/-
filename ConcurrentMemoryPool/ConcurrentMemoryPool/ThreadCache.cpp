#include"ThreadCache.h"
#include"CentralCache.h"

//�����Ŀ�ȡ�ڴ�
void* ThreadCache::FetchFromCentralCache(size_t index, size_t bytes)
{
	FreeList* freelist = &_freelist[index];
	size_t num_to_move = min(ClassSize::NumMoveSize(bytes), freelist->MaxSize());//num_to_move����Ҫ�������ĸ���
	void *start, *end;
	size_t fetchnum = CentralCache::GetInstance()->FetchRangeObj(start, end, num_to_move, bytes);//fetchnum��ʵ������Ķ������
	if (fetchnum > 1)
	{
		freelist->PushRange(NEXT_OBJ(start), end, fetchnum - 1);  //�����˵�һ���ڴ��������ڴ��ҵ�freelist�£�����һ������
	}

	if (num_to_move == freelist->MaxSize())
	{
		int tmp = num_to_move;
		freelist->SetMaxSize(++tmp);
	}

	return start;  //����һ������
}

//����size�ֽڴ�С���ڴ�
void* ThreadCache::Allocate(size_t size)
{
	assert(size <= MAXBYTES);

	//����ȡ��
	size = ClassSize::Roundup(size);
	//����ȡ�����ڴ���Ӧfreelist�����е�λ��
	size_t index = ClassSize::Index(size);
	FreeList* freelist = &_freelist[index];
	if (!freelist->Empty())
	{
		return freelist->Pop();//ȡ����Ӧλ���ڴ�鲢����
	}
	else
	{
		return FetchFromCentralCache(index, size);//�����Ŀ�ȡ�ڴ�
	}
}

//���������¹��صĶ���̫�࣬��ʼ����
void ThreadCache::ListTooLong(FreeList* freelist, size_t bytes)
{
	void* start = freelist->Clear();
	CentralCache::GetInstance()->ReleaseListToSpan(start, bytes);
}

//�ͷ��ڴ棨���գ�
void ThreadCache::Deallocate(void* ptr, size_t bytes)
{
	assert(bytes <= MAXBYTES);
	bytes = ClassSize::Roundup(bytes);
	size_t index = ClassSize::Index(bytes);
	FreeList* freelist = &_freelist[index];
	freelist->push(ptr);

	//�����������еĶ������������CentralCacheһ���Ի�ȡ�Ķ������ʱ
	//��ʼ���ն���CentralCache
	if (freelist->Size() >= freelist->MaxSize())
	{
		ListTooLong(freelist, bytes);
	}
}