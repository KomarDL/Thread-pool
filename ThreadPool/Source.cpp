#include <iostream>
#include "pool.hpp"
using namespace Pool;

void Add(PVOID pvData)
{
	PINT i = static_cast<PINT>(pvData);
	*i += 10;
	printf_s("%d\n", *i);
}

int main()
{
	auto p = new ThreadPool(4);
	p->start();
	for (size_t i = 0; i < 10; i++)
	{
		p->queueUserWorkItem(Add, new int(i));
	}
	p->waitAll();
	puts("\n\n");
	for (size_t i = 0; i < 30; i++)
	{
		p->queueUserWorkItem(Add, new int(i));
	}
	std::cin.get();
	return 0;
}