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

int LogLevel = LOG_DEBUG;//Ĭ����־����

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
		(NULL, "����Ŀ¼ʧ��", NULL, MB_OK);
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
		//����ļ���СΪ0����Ϊ�´������ļ�����д��UTF-8�ļ�ͷ
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
	//��ת��UNICODE
	int nLen = MultiByteToWideChar(CP_ACP, 0, msg, -1, NULL, 0);
	WCHAR* pwszBuffer = new WCHAR[nLen];
	nLen = MultiByteToWideChar(CP_ACP, 0, msg, -1, pwszBuffer, nLen);

	//�ٽ�UNICODEת����UTF-8����
	nLen = WideCharToMultiByte(CP_UTF8, 0, pwszBuffer, -1, NULL, 0, NULL, NULL);
	char* pszBuffer = new char[nLen];
	nLen = WideCharToMultiByte(CP_UTF8, 0, pwszBuffer, -1, pszBuffer, nLen, NULL, NULL);
	delete[] pwszBuffer;
	DWORD written = 0;
	WriteFile(fp, pszBuffer, strlen(pszBuffer), &written, NULL);
	delete[] pszBuffer;
}

Logger* Logger::_instance = NULL;

//������־����
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

//��ʼ���ڴ�Ĵ�С
int BUFFER_SIZE = 2048;
//���棬���ڻ�����־����
char* buffer = NULL;

void __stdcall log(int level, const char* fmt, ...)
{
	if (level <= LogLevel)
	{
		//���û�г�ʼ�������ʼ������
		if (NULL == buffer)
		{
			buffer = new char[BUFFER_SIZE];
		}
		memset(buffer, 0, BUFFER_SIZE);
		va_list va;
		va_start(va, fmt);
		int result = vsnprintf(buffer, BUFFER_SIZE - 1, fmt, va);
		//result = -1��ʾ�����С�����Է�����־����
		bool expandedSize = false;
		while (result == -1)
		{
			//���治�㣬��ɾ��ԭ���Ļ���
			delete[] buffer;
			//������Ĵ�С����
			BUFFER_SIZE = 2 * BUFFER_SIZE;
			//��ʼ������
			buffer = new char[BUFFER_SIZE];
			memset(buffer, 0, BUFFER_SIZE);
			result = vsnprintf(buffer, BUFFER_SIZE - 1, fmt, va);
			expandedSize = true;
		}
		va_end(va);
		buffer[result + 1] = 0;
		string msg(buffer);
		string loglevel = getLogLevel(level);

		//��־���������
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

		//���һ���ַ����ǻ��з�
		if (msg.at(msg.length() - 1) != '\n')
		{
			//������һ���ַ�ʱ�س����򽫻س�ɾ��
			if (msg.at(msg.length() - 1) == '\r')
			{
				msg.erase(msg.length() - 1, 1);
			}
			//�Ǿ��������ϻس�����
			msg += "\r\n";
		}
		//�����ڶ������ǻس��ַ�
		else if (msg.at(msg.length() - 2) != '\r')
		{
			//�����з�ɾ��
			msg.erase(msg.length() - 1, 1);
			//�������ϻس�����
			msg += "\r\n";
		}
		if (Logger::getInstance() != NULL)
		{
			Logger::getInstance()->writeLog(msg);
		}
	}
}