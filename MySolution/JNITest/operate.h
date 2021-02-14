#pragma once
#ifndef __JNI_OPERATE_
#define __JNI_OPERATE_

#include <iostream>
#include <windows.h>
#include <string.h>
#include "com_yxh_jni_Test.h"

using namespace std;

//д��־����ָ��
typedef void (*Plog)(int, const char*, ...);

//�������namespace�У��Ա���JNITest�е��ø���
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

#define LOG_ERROR 1//ERROR������־
#define LOG_INFO 2//INFO������־
#define LOG_DEBUG 3 //DEBUG������־

//���ڷ���ʱ��ʶ�����Ƿ�ɹ�
#define SUCCFLAG 0//�ɹ�
#define FAILFLAG -1//ʧ��

#endif