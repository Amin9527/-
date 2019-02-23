#include"ThreadCache.h"

#define SUM 10

// Õ∑≈≤‚ ‘
void Deallocate()
{

}

//…Í«Î≤‚ ‘
void AllocTest(int bytes)
{
	ThreadCache t;
	
	std::vector<void*> v;


	for (size_t i = 0; i < SUM; ++i)
	{
		v.push_back(t.Allocate(bytes));
		cout << v[i] << endl;
	}

	for (size_t i = 0; i < SUM; ++i)
	{
		t.Deallocate(v[i], bytes);
	}

	cout << endl;

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

}

int main()
{
	int bytes = 6;
	AllocTest(bytes);
	cout << endl;
	system("pause");
	return 0;
}