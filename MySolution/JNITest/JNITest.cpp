#pragma once

#include "operate.h"

//写日志函数
Plog plog = NULL;

void setFunctionReturn(JNIEnv* env, jobjectArray resArr, int ret, char* content, const char* msg, ...)
{
	//初始化错误信息缓存的大小
	int BUFFER_SIZE = 512;
	//错误信息的缓存
	char* buffer = new char[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);

	const char* msgReadl = "";
	if (strcmp(msg, "") != 0) {
		//获取错误信息
		va_list va;
		va_start(va, msg);
		int result = vsnprintf(buffer, BUFFER_SIZE - 1, msg, va);
		if (result == -1) {
			//缓存不足，则删除原来的缓存
			delete[] buffer;
			//将缓存大小翻倍
			BUFFER_SIZE = 2 * BUFFER_SIZE;
			//初始化缓存
			buffer = new char[BUFFER_SIZE];
			memset(buffer, 0, BUFFER_SIZE);
			result = vsnprintf(buffer, BUFFER_SIZE - 1, msg, va);
		}
		va_end(va);
		buffer[result + 1] = 0;
		msgReadl = buffer;
	}

	//以java的HashMap返回至java
	jclass java_cls_HashMap = env->FindClass("java/util/HashMap");
	jmethodID java_mid_HashMap = env->GetMethodID(java_cls_HashMap, "<init>", "()V");
	jobject java_obj_HashMap = env->NewObject(java_cls_HashMap, java_mid_HashMap, "");
	jmethodID java_mid_HashMap_put = env->GetMethodID(java_cls_HashMap, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	char retStr[4] = { 0 };
	_itoa_s(ret, retStr, sizeof(retStr), 10);
	env->CallObjectMethod(java_obj_HashMap, java_mid_HashMap_put, env->NewStringUTF("errCode"), env->NewStringUTF(retStr));
	if (ret == 0) {
		env->CallObjectMethod(java_obj_HashMap, java_mid_HashMap_put, env->NewStringUTF("content"), env->NewStringUTF(content));
	} else {
		env->CallObjectMethod(java_obj_HashMap, java_mid_HashMap_put, env->NewStringUTF("msg"), env->NewStringUTF(msgReadl));
	}
	env->SetObjectArrayElement(resArr, 0, java_obj_HashMap);
}

JNIEXPORT jint JNICALL Java_com_yxh_jni_Test_callC(JNIEnv* env, jobject obj, jbyteArray methodName, jobjectArray paramArr, jobjectArray resArr)
{
	HINSTANCE hLib = LoadLibrary("./dll/Logger.dll");
	if (hLib == NULL)
	{
		MessageBox(NULL, "加载日志功能动态库失败", NULL, MB_OK);
		setFunctionReturn(env, resArr, -1, (char*)"", "加载日志功能动态库失败");
		return 0;
	}
	plog = (Plog)GetProcAddress(hLib, "log");
	if (plog == NULL)
	{
		MessageBox(NULL, "加载日志记录函数失败", NULL, MB_OK);
		setFunctionReturn(env, resArr, -1, (char*)"", "加载日志记录函数失败");
		return 0;
	}

	char* pMethod;
	jint methodSize = env->GetArrayLength((jarray)methodName);
	//取方法名
	pMethod = (char*)malloc(methodSize + 1);
	memset(pMethod, 0x00, _msize(pMethod));
	env->GetByteArrayRegion(methodName, 0, methodSize, (signed char*)pMethod);

	//把方法名全部改为小写
	for (int i = 0; i < _msize(pMethod); i++)
	{
		pMethod[i] = tolower(pMethod[i]);
	}

	int ret = -999;
	if (!strcmp(pMethod, "say"))
	{
		jsize paramSize = env->GetArrayLength(paramArr);
		if (paramSize < 0)
		{
			jstring jstr = Operate::getInstance()->WindowsTojstring(env, (char*)"参数个数小于0个");
			const char* msg = env->GetStringUTFChars(jstr, JNI_FALSE);
			setFunctionReturn(env, resArr, -1, (char*)"", msg);
			return -1;
		}
		
		for (int i = 0; i < paramSize; i++) {
			char* param = Operate::getInstance()->jstringToWindows(env, (jstring)env->GetObjectArrayElement(paramArr, i));
			plog(LOG_INFO, "param%d=%s", i, param);
			int ret = Operate::getInstance()->say(i + 1, param);
		}
		setFunctionReturn(env, resArr, 0, (char*)"0", "");
	}
	return 0;
}
