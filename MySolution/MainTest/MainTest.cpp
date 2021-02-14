#pragma once
#include <iostream>
#include <Windows.h>
#include "logger.h"

#define LOG_ERROR 1//ERROR级别日志
#define LOG_INFO 2//INFO级别日志
#define LOG_DEBUG 3 //DEBUG级别日志

int main()
{
	//plog(LOG_DEBUG, "test");
	log(LOG_DEBUG, "test");
    std::cout << "Hello World!\n";
}
