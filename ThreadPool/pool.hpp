#pragma once

#include <Windows.h>
#include <vector>
#include "../Concurrent queue/queue.hpp"

namespace Pool
{
	using WorkItem = void(*)(PVOID);
	using Concurrent::ConcurrentQueue;
	using std::vector;
	using std::unique_ptr;

	class ThreadPool
	{
	public:
		static constexpr auto DEFAULT_THREAD_AMOUNT = 5;
		explicit ThreadPool(ULONG threadAmount = DEFAULT_THREAD_AMOUNT);
		~ThreadPool();

		void start() noexcept;
		bool queueUserWorkItem(WorkItem wi, PVOID data) noexcept { return queue->enqueue(Task(wi, data)); }
		void waitAll() noexcept;

	private:
		typedef struct _Task
		{
			WorkItem wi;
			PVOID param;
			_Task(WorkItem workItem, PVOID param = nullptr) 
				: wi(workItem)
				, param(param)
			{
			}
		} Task;

		unique_ptr<ConcurrentQueue<Task>> queue;
		unique_ptr<vector<HANDLE>> threads;
		bool stop;
		bool wait;
		SYNCHRONIZATION_BARRIER barrier;

		void allocateThreads();
		static DWORD WINAPI threadProc(ThreadPool *threadPool);
	};
}