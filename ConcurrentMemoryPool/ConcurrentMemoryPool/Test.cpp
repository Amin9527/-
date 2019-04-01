#include"ThreadCache.h"
#include<thread>
#include<time.h>
#include<Windows.h>

#define SUM 10000

#define COUNT 10
#define THREAD_SUM 20

std::vector<void*> v(COUNT);


/*
//申请测试
void AllocTest(int bytes)
{
	ThreadCache t;
	void *ptr = t.Allocate(bytes);
	cout << ptr << endl;
}

void fun(int bytes)
{
	for (int i = 0; i < COUNT; ++i)
	{
		AllocTest(bytes);
	}
}
*/

void AllocTest(int bytes);
void MallocTest(int bytes);
int main()
{
	std::vector<std::thread> v(THREAD_SUM);
	size_t begin1 = clock();
	for (int i = 0; i < THREAD_SUM; i++)
	{
		v[i]= std::thread(AllocTest, 600);
		//Sleep(500);
	}
	size_t end1 = clock();

	std::vector<std::thread> vv(THREAD_SUM);
	size_t begin2 = clock();
	for(int i = 0; i < THREAD_SUM; ++i)
	{
		vv[i] = std::thread(MallocTest, 600);
	}
	size_t end2 = clock();
	for (auto &e : v)
	{
		e.join();
	}
	for (auto &e : vv)
	{
		e.join();
	}
	size_t time1 = end1 - begin1;
	size_t time2 = end2 - begin2;
	cout << time1 << endl << time2 << endl;

	cout << endl;
	system("pause");
	return 0;
}

//Malloc申请测试
void MallocTest(int bytes)
{
	
	for (size_t i = 0; i < SUM; ++i)
	{
		void *ptr = malloc(bytes);
	}
	
}

//CurrentAlloc申请测试
void AllocTest(int bytes)
{
	ThreadCache t;

	std::vector<void*> v;

	
	for (size_t i = 0; i < SUM; ++i)
	{
		t.Allocate(bytes);
		//v.push_back(t.Allocate(bytes));
		//cout << v[i] << endl;
	}
	

	for (size_t i = 0; i < SUM; ++i)
	{
		//t.Deallocate(v[i], bytes);
	}


	

	/*
	std::vector<void*> vv;

	for (size_t i = 0; i < SUM; ++i)
	{
		vv.push_back(t.Allocate(bytes));
		cout << vv[i] << endl;
	}

	for (size_t i = 0; i < SUM; ++i)
	{
		t.Deallocate(vv[i], bytes);
	}
	cout << endl;
	*/
}

/*
int main()
{
	int bytes = 6;
	AllocTest(bytes);
	cout << endl;
	system("pause");
	return 0;
}
*/

//#include "Common.hpp"
//#include "ConcurrentAlloc.hpp"
//#include <vector>

/*
#define MAXSIZE 6

// 单轮次申请释放次数 线程数 轮次
void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	ThreadCache t;
	std::vector<std::thread> vthread(nworks);
	size_t malloc_costtime = 0;
	size_t free_costtime = 0;
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			std::vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v.push_back(t.Allocate(MAXSIZE));
				}
				size_t end1 = clock();
				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					t.Deallocate(v[i],MAXSIZE);
				}
				size_t end2 = clock();
				v.clear();

				malloc_costtime += end1 - begin1;
				free_costtime += end2 - begin2;
			}
		});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%u个线程并发执行%u轮次，每轮次concurrent alloc %u次: 花费：%u ms\n", nworks, rounds, ntimes, malloc_costtime);
	printf("%u个线程并发执行%u轮次，每轮次concurrent dealloc %u次: 花费：%u ms\n", nworks, rounds, ntimes, free_costtime);
	printf("%u个线程并发concurrent alloc&free %u次，总计花费：%u ms\n", nworks, nworks*rounds*ntimes, malloc_costtime + free_costtime);
}

void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	size_t malloc_costtime = 0;
	size_t free_costtime = 0;
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			std::vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v.push_back(malloc(MAXSIZE));
				}
				size_t end1 = clock();
				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					free(v[i]);
				}
				size_t end2 = clock();
				v.clear();

				malloc_costtime += end1 - begin1;
				free_costtime += end2 - begin2;
			}
		});
	}
	for (auto& t : vthread)
	{
		t.join();
	}
	printf("%u个线程并发执行%u轮次，每轮次malloc %u次: 花费：%u ms\n", nworks, rounds, ntimes, malloc_costtime);
	printf("%u个线程并发执行%u轮次，每轮次free %u次: 花费：%u ms\n", nworks, rounds, ntimes, free_costtime);
	printf("%u个线程并发concurrent malloc&free %u次，总计花费：%u ms\n", nworks, nworks*rounds*ntimes, malloc_costtime + free_costtime);
}

int main()
{
	printf("==============每轮申请的内存是：%ubyte==============\n", MAXSIZE);
	std::cout << "===========================================================================" << std::endl;
	BenchmarkMalloc(100, 50, 100);
	std::cout << "===========================================================================" << std::endl;
	std::cout << "===========================================================================" << std::endl;
	BenchmarkConcurrentMalloc(100, 50, 100);
	std::cout << "===========================================================================" << std::endl;

	return 0;
}
*/