#include <Windows.h>
#include <queue>
#include <string>

class BlockQueue
{
public:
	BlockQueue();
	~BlockQueue();

	void put(std::string msg);
	std::string take();

	//获取队列中消息的数量
	const int getCount() const;
private:
	std::queue<std::string> msgQueue;
	int count;
	HANDLE _MUTEX_PUT;
	HANDLE _MUTEX_TAKE;
	HANDLE _MUTEX_COUNT;
};