#include <Windows.h>
#include <string.h>

#include "BlockQueue.h"

#define LOG_ERROR 1//ERROR������־
#define LOG_INFO 2//INFO������־
#define LOG_DEBUG 3 //DEBUG������־

using namespace std;

#pragma warning(disable:4996)

void setLogLevel(int level);
std::string getLogLevel(int level);
void __stdcall log(int level, const char* fmt, ...);

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
	static unsigned __stdcall startLogger(void* args);
	void outPutLog(const char* msg);
	static Logger* _instance;
	BlockQueue* _blockQueue;
	HANDLE thread;
	HANDLE fp;
};