#include <Windows.h>
#include <string.h>

#include "BlockQueue.h"

#define LOG_ERROR 1//ERROR级别日志
#define LOG_INFO 2//INFO级别日志
#define LOG_DEBUG 3 //DEBUG级别日志

using namespace std;

#pragma warning(disable:4996)

void __stdcall setLogLevel(int level);
std::string __stdcall getLogLevel(int level);
void __stdcall dslog(int level, const char* fmt, ...);

class Logger
{
public:
	static Logger* getInstance();
	static void releaseInstance();
	void writeLog(std::string msg);
private:
	Logger();
	Logger(const Logger&);
	Logger& operator = (const Logger& a);
	~Logger();
	bool startLogThread();
	static unsigned startLogger(void* args);
	void outPutLog(const char* msg);
	static Logger* _instance;
	BlockQueue* _blockQueue;
	HANDLE thread;
	HANDLE fp;
};