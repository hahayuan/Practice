#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <time.h>
#include <direct.h>
#include <string>
#include <stdexcept>
#include <process.h>
#include <atltime.h>
#include <share.h>

#include "logger.h"

int LogLevel = LOG_DEBUG;//默认日志级别

using namespace std;


#pragma warning(disable:4996)

Logger* Logger::getInstance()
{
	if (_instance == NULL)
	{
		try
		{
			_instance = new Logger();
			_instance->startLogThread();
		}
		catch (runtime_error&)
		{
			return NULL;
		}
		catch (...)
		{
			return NULL;
		}
	}
	return _instance;
}

void Logger::releaseInstance()
{
	if (_instance != NULL)
		delete _instance;
	_instance = NULL;
}

void Logger::writeLog(string msg)
{
	if (_instance != NULL)
	{
		_blockQueue->put(msg);
	}
}

Logger::Logger()
{
	_blockQueue = NULL;
	thread = NULL;
	fp = NULL;
	char fileName[50] = "";
	time_t t;
	time(&t);
	struct  tm* localTime;
	localTime = localtime(&t);
	int retMkdir = _mkdir("log");
	if (retMkdir != 0) {
		(NULL, "创建目录失败", NULL, MB_OK);
	}
	sprintf(fileName, "./log/Dll_Log_%04d%02d%02d.log",
		1900 + localTime->tm_year,
		localTime->tm_mon + 1,
		localTime->tm_mday);
	fp = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
	if (fp == NULL || fp == INVALID_HANDLE_VALUE)
	{
		fp = NULL;
		throw runtime_error("Logger failed to create log file.");
	}
	LARGE_INTEGER size;
	if (!GetFileSizeEx(fp, &size))
	{
		throw runtime_error("Logger failed to open log file.");
	}
	if (size.QuadPart == 0)
	{
		//如果文件大小为0，即为新创建的文件，则写入UTF-8文件头
		DWORD written = 0;
		unsigned char pszBuffer[] = {0xEF, 0xBB, 0xBF, 0x00};
		WriteFile(fp, pszBuffer, 3, &written, NULL);
	}
	SetFilePointer(fp, 0, 0, FILE_END);
	_blockQueue = new BlockQueue();
}

Logger::Logger(const Logger&)
{
}

Logger& Logger::operator = (const Logger& a)
{
	this->_blockQueue = a._blockQueue;
	this->thread = a.thread;
	this->fp = a.fp;
	return *this;
}

Logger::~Logger()
{
	if (fp != NULL)
	{
		CloseHandle(fp);
		fp = NULL;
	}
	if (_blockQueue != NULL)
	{
		delete _blockQueue;
	}
	if (thread != NULL)
	{
		CloseHandle(thread);
		thread = NULL;
	}
}

bool Logger::startLogThread()
{
	thread = (HANDLE)_beginthreadex(NULL, 0, &startLogger, NULL, 0, NULL);
	if (thread != NULL)
		return true;
	return false;
}

unsigned __stdcall Logger::startLogger(void* args)
{
	for (;;)
	{
			string msg = Logger::getInstance()->_blockQueue->take();
			Logger::getInstance()->outPutLog(msg.c_str());
	}
	return 0;
}

void Logger::outPutLog(const char* msg)
{
	//先转成UNICODE
	int nLen = MultiByteToWideChar(CP_ACP, 0, msg, -1, NULL, 0);
	WCHAR* pwszBuffer = new WCHAR[nLen];
	nLen = MultiByteToWideChar(CP_ACP, 0, msg, -1, pwszBuffer, nLen);

	//再将UNICODE转换成UTF-8编码
	nLen = WideCharToMultiByte(CP_UTF8, 0, pwszBuffer, -1, NULL, 0, NULL, NULL);
	char* pszBuffer = new char[nLen];
	nLen = WideCharToMultiByte(CP_UTF8, 0, pwszBuffer, -1, pszBuffer, nLen, NULL, NULL);
	delete[] pwszBuffer;
	DWORD written = 0;
	WriteFile(fp, pszBuffer, strlen(pszBuffer), &written, NULL);
	delete[] pszBuffer;
}

Logger* Logger::_instance = NULL;

//设置日志级别
void setLogLevel(int level)
{
	LogLevel = level;
}

string getLogLevel(int level)
{
	if (level == LOG_DEBUG)
	{
		return "[DEBUG]";
	}
	else if (level == LOG_INFO)
	{
		return "[INFO]";
	}
	else
	{
		return "[ERROR]";
	}
}

//初始化内存的大小
int BUFFER_SIZE = 2048;
//缓存，用于缓存日志内容
char* buffer = NULL;

void __stdcall log(int level, const char* fmt, ...)
{
	if (level <= LogLevel)
	{
		//如果没有初始化，则初始化缓存
		if (NULL == buffer)
		{
			buffer = new char[BUFFER_SIZE];
		}
		memset(buffer, 0, BUFFER_SIZE);
		va_list va;
		va_start(va, fmt);
		int result = vsnprintf(buffer, BUFFER_SIZE - 1, fmt, va);
		//result = -1表示缓存大小不足以放下日志内容
		bool expandedSize = false;
		while (result == -1)
		{
			//缓存不足，则删除原来的缓存
			delete[] buffer;
			//将缓存的大小翻倍
			BUFFER_SIZE = 2 * BUFFER_SIZE;
			//初始化缓存
			buffer = new char[BUFFER_SIZE];
			memset(buffer, 0, BUFFER_SIZE);
			result = vsnprintf(buffer, BUFFER_SIZE - 1, fmt, va);
			expandedSize = true;
		}
		va_end(va);
		buffer[result + 1] = 0;
		string msg(buffer);
		string loglevel = getLogLevel(level);

		//日志中输出毫秒
		string time = "";
		SYSTEMTIME sysTime;
		GetLocalTime(&sysTime);
		const int nBufSize = 256;
		TCHAR chBuf[nBufSize];
		memset(chBuf, 0x00, sizeof(chBuf));
		wsprintf(chBuf, _T("%u-%02u-%02u %02u:%02u:%02u.%03u"),
			sysTime.wYear, sysTime.wMonth, sysTime.wDay,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond,
			sysTime.wMilliseconds);
		time = chBuf;

		char threadId[20] = "";
		sprintf(threadId, "%05d", (unsigned)GetCurrentThreadId());
		string expandLabel;
		if (expandedSize)
		{
			memset(chBuf, 0, sizeof(chBuf));
			wsprintf(chBuf, _T("[Expanded Buffer Size to %d.]"), BUFFER_SIZE);
			expandLabel = chBuf;
		}
		msg = time + " [Thread-" + threadId + "]" + loglevel + expandLabel + " " + msg;

		//最后一个字符不是换行符
		if (msg.at(msg.length() - 1) != '\n')
		{
			//如果最后一个字符时回车，则将回车删除
			if (msg.at(msg.length() - 1) == '\r')
			{
				msg.erase(msg.length() - 1, 1);
			}
			//那就在最后加上回车换行
			msg += "\r\n";
		}
		//倒数第二个不是回车字符
		else if (msg.at(msg.length() - 2) != '\r')
		{
			//将换行符删除
			msg.erase(msg.length() - 1, 1);
			//在最后加上回车换行
			msg += "\r\n";
		}
		if (Logger::getInstance() != NULL)
		{
			Logger::getInstance()->writeLog(msg);
		}
	}
}