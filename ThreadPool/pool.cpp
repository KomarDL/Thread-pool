#include "pool.hpp"
#include <exception>
#include <stdexcept>

Pool::ThreadPool::ThreadPool(ULONG threadAmount)
	: queue(new ConcurrentQueue<Task>)
	, threads(new vector<HANDLE>(threadAmount))
	, stop(false)
	, wait(false)
{
	if (!InitializeSynchronizationBarrier(&barrier, threadAmount, -1))
		throw std::exception("Unable to initialize synchronization barrier");
	
	try
	{
		allocateThreads();
	}
	catch (const std::exception& e)
	{
		DeleteSynchronizationBarrier(&barrier);
		throw e;
	}
}

Pool::ThreadPool::~ThreadPool()
{
	stop = true;
	wait = true;
	start();
	WaitForMultipleObjects(threads->size(), threads->data(), true, INFINITE);
}

inline void Pool::ThreadPool::start() noexcept
{
	for(auto thread : *threads)
	{
		ResumeThread(thread);
	}
}

void Pool::ThreadPool::waitAll() noexcept
{
	wait = true;
	while (wait)
	{
		SwitchToThread();
	}
}

void Pool::ThreadPool::allocateThreads()
{
	for (auto i = 0u; i < threads->size(); ++i)
	{
		(*threads)[i] = CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(threadProc), this, CREATE_SUSPENDED, NULL);
		if ((*threads)[i] == nullptr)
		{
			for (auto j = 0u; j < i; ++j)
			{
				TerminateThread((*threads)[j], -1);
			}
			throw std::exception("Unable to create a given number of threads");
		}
	}
}

DWORD WINAPI Pool::ThreadPool::threadProc(ThreadPool *threadPool)
{
	while (!threadPool->stop)
	{
		while (!threadPool->wait || !threadPool->queue->isEmpty())
		{
			try
			{
				auto task = threadPool->queue->dequeue();
				task.wi(task.param);
			}
			catch (const std::exception&)
			{
				SwitchToThread();
			}
		}
		EnterSynchronizationBarrier(&(threadPool->barrier), 0);
		threadPool->wait = false;
	}
	return 0;
}
