#pragma once
#include <Windows.h>
#include <memory>
#include <stdexcept>
#include <exception>

namespace Concurrent
{
	template<typename T>
	class ConcurrentQueue
	{
	public:
		static constexpr DWORD DEFAULT_SPIN_COUNT = 4000;

		explicit ConcurrentQueue(DWORD spinCount = DEFAULT_SPIN_COUNT);
		~ConcurrentQueue();

		bool isEmpty();
		bool enqueue(T data);
		T dequeue() throw(...); 

	private:

		template<typename TT>
		struct _TNode
		{
			TT data;
			struct _TNode* next;

			_TNode(TT data, struct _TNode* next = nullptr)
				: data(data), next(next)
			{
			}
		};
		typedef struct _TNode<T> Node, *PNode;

		CRITICAL_SECTION cs;
		PNode first;
		PNode last;
		LONG length;
	};

	template<typename T>
	inline ConcurrentQueue<T>::ConcurrentQueue(DWORD spinCount)
		: length{}
		, first{}
		, last{}
	{
		(void)InitializeCriticalSectionAndSpinCount(&cs, spinCount);
	}

	template<typename T>
	inline bool ConcurrentQueue<T>::isEmpty()
	{
		return length == 0;
	}

	template<typename T>
	bool ConcurrentQueue<T>::enqueue(T data)
	{
		PNode newNode = new Node(data);

		EnterCriticalSection(&cs);
		if (0 == length)
		{
			first = newNode;
		}
		else
		{
			last->next = newNode;
		}
		last = newNode;

		LeaveCriticalSection(&cs);
		InterlockedIncrement(&length);

		return true;
	}

	// throw std::exception
	template<typename T>
	T ConcurrentQueue<T>::dequeue() throw(...) 
	{
		if (length == 0)
			throw std::exception("queue is empty");

		EnterCriticalSection(&cs);

		if (length == 0)
		{
			LeaveCriticalSection(&cs);
			throw std::exception("queue is empty");
		}

		T result = first->data;
		PNode tmp = first;
		first = first->next;
		delete tmp;

		if (InterlockedDecrement(&length) == 0)
		{
			last = nullptr;
		}
		LeaveCriticalSection(&cs);

		return result;
	}

	template<typename T>
	ConcurrentQueue<T>::~ConcurrentQueue()
	{
		DeleteCriticalSection(&cs);
		while (first != nullptr)
		{
			PNode tmp = first;
			first = first->next;
			delete tmp;
		}
	}
}