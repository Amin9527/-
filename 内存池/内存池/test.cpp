#include"ThreadCache.h"

void AllocTest(int bytes)
{
	ThreadCache t;
	
	for (size_t i = 0; i < 1030; ++i)
	{
		int *p = (int*)t.Allocate(bytes);
		cout << p << endl;
	}
}

int main()
{
	int bytes = 6;
	AllocTest(bytes);
	system("pause");
	return 0;
}