#include <stdexcept>

#include "BlockQueue.h"

using namespace std;

BlockQueue::BlockQueue()
{
	_MUTEX_PUT = CreateMutex(NULL, FALSE, NULL);
	if (_MUTEX_PUT == NULL)
	{
		throw runtime_error("Logger failed to create mutex_put.");
	}

	_MUTEX_TAKE = CreateSemaphore(NULL, 0, 1, NULL);
	if (_MUTEX_TAKE == NULL)
	{
		CloseHandle(_MUTEX_PUT);
		_MUTEX_PUT = NULL;
		throw runtime_error("Logger failed to create mutex_take.");
	}

	_MUTEX_COUNT = CreateMutex(NULL, FALSE, NULL);
	if (_MUTEX_COUNT == NULL)
	{
		CloseHandle(_MUTEX_PUT);
		_MUTEX_PUT = NULL;
		CloseHandle(_MUTEX_TAKE);
		_MUTEX_TAKE = NULL;
		throw runtime_error("Logger failed to create mutex_count.");
	}
	count = 0;
}
BlockQueue::~BlockQueue()
{
	if (_MUTEX_PUT != NULL)
	{
		CloseHandle(_MUTEX_PUT);
	}
	if (_MUTEX_TAKE != NULL)
	{
		CloseHandle(_MUTEX_TAKE);
	}
	if (_MUTEX_COUNT != NULL)
	{
		CloseHandle(_MUTEX_COUNT);
	}
}

void BlockQueue::put(string msg)
{
	if (WAIT_OBJECT_0 == WaitForSingleObject(_MUTEX_PUT, INFINITE))
	{
		WaitForSingleObject(_MUTEX_COUNT, INFINITE);
		msgQueue.push(msg);
		++count;
		if (count == 1)
		{

			ReleaseSemaphore(_MUTEX_TAKE, 1, 0);
		}
		ReleaseMutex(_MUTEX_COUNT);
		ReleaseMutex(_MUTEX_PUT);
	}
}

string BlockQueue::take()
{
_begintake:
	WaitForSingleObject(_MUTEX_COUNT, INFINITE);
	if (count <= 0)
	{
		ReleaseMutex(_MUTEX_COUNT);
		WaitForSingleObject(_MUTEX_TAKE, INFINITE);
		goto _begintake;
	}
	string msg = msgQueue.front();
	msgQueue.pop();
	--count;
	ReleaseMutex(_MUTEX_COUNT);
	return msg;
}

//获取队列中消息的数量
const int BlockQueue::getCount() const {
	WaitForSingleObject(_MUTEX_COUNT, INFINITE);
	int tmp = count;
	ReleaseMutex(_MUTEX_COUNT);
	return tmp;
}