#pragma once
#include <iostream>
#include <Windows.h>

#define LOG_ERROR 1//ERROR级别日志
#define LOG_INFO 2//INFO级别日志
#define LOG_DEBUG 3 //DEBUG级别日志

//写日志函数指针
typedef void (*Plog)(int, const char*, ...);

int main()
{
	HMODULE hModule = LoadLibrary(".\\dll\\Logger.dll");
	if (hModule == NULL)
	{
		MessageBox(NULL, "加载日志功能动态库失败", NULL, MB_OK);
		return -1;
	}
	//写日志函数
	Plog plog = (Plog)GetProcAddress(hModule, "dslog");
	if (plog == NULL)
	{
		MessageBox(NULL, "加载日志记录函数失败", NULL, MB_OK);
		return -1;
	}

	plog(LOG_DEBUG, "test");
	FreeLibrary(hModule);
	return 0;
}