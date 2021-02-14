#pragma once
//#pragma warning(disable:4244)

#include "operate.h"

Operate* Operate::instance = NULL;
Plog Operate::log = NULL;

Operate* Operate::getInstance()
{
	HINSTANCE hLib = LoadLibrary("./dll/Logger.dll");
	if (hLib == NULL)
	{
		MessageBox(NULL, "加载日志功能动态库失败", NULL, MB_OK);
	}
	log = (Plog)GetProcAddress(hLib, "log");
	if (log == NULL)
	{
		MessageBox(NULL, "加载日志记录函数失败", NULL, MB_OK);
	}

	if (instance == NULL)
	{
		instance == new Operate();
	}
	return instance;
}

int Operate::say(int i, char* params)
{
	log(LOG_DEBUG, "say()");
	printf("我想说的第%d句话：%s\n", i, params);
	return 0;
}

//将jstring类型转换为windows类型
char* Operate::jstringToWindows(JNIEnv* env, jstring jstr)
{
	jsize len = env->GetStringLength(jstr);
	const jchar* jcstr = env->GetStringChars(jstr, 0);
	int size = 0;
	char* str = (char*)malloc(len * 2 + 1);
	size = WideCharToMultiByte(CP_ACP, 0, LPCWSTR(jcstr), len, str, len * 2 + 1, NULL, NULL);
	if (size <= 0)
		return NULL;
	env->ReleaseStringChars(jstr, jcstr);
	return str;
}

//将windows类型转换成jstring类型
jstring Operate::WindowsTojstring(JNIEnv* env, char* str)
{
	jstring rtn = 0;
	int slen = strlen(str);
	unsigned short* buffer = 0;
	if (slen == 0)
		rtn = env->NewStringUTF(str);
	else
	{
		int length = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str, slen, NULL, 0);
		buffer = (unsigned short*)malloc(length * 2 + 1);
		if (MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str, slen, (LPWSTR)buffer, length) > 0)
			rtn = env->NewString((jchar*)buffer, length);
	}
	if (buffer)
		free(buffer);
	return rtn;
}