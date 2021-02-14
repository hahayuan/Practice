#pragma once
#ifndef __JNI_OPERATE_
#define __JNI_OPERATE_

#include <iostream>
#include <windows.h>
#include <string.h>
#include "com_yxh_jni_Test.h"

using namespace std;

//写日志函数指针
typedef void (*Plog)(int, const char*, ...);

//将类放在namespace中，以便在JNITest中调用该类
namespace _Operate
{
	class Operate
	{
	public:
		~Operate() {};
		static Operate* getInstance();
		int say(int i, char* params);
		char* jstringToWindows(JNIEnv* env, jstring jstr);
		jstring WindowsTojstring(JNIEnv* env, char* str);
		
	private:
		Operate() {};
		static Operate* instance;
		static Plog log;
	};
}

using namespace _Operate;

#define LOG_ERROR 1//ERROR级别日志
#define LOG_INFO 2//INFO级别日志
#define LOG_DEBUG 3 //DEBUG级别日志

//用于返回时标识处理是否成功
#define SUCCFLAG 0//成功
#define FAILFLAG -1//失败

#endif