#pragma once
#include <iostream>
#include <sstream>
#include <Windows.h>

#define LOG_COLOR_DEFAULT 7
#define LOG_COLOR_GREEN 10
#define LOG_COLOR_YELLOW 14
#define LOG_COLOR_RED 12

class Logger
{
public:
	static void Init();

	static void Log(const std::string& msg);
	static void LogSuccess(const std::string& msg);
	static void LogWarning(const std::string& msg);
	static void LogError(const std::string& msg);

private:
	static void LogColor(const std::string& msg, int color);

private:
	static HANDLE hConsole;
};
