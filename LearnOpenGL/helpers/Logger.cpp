#include "Logger.h"
#include <sstream>

HANDLE Logger::hConsole;

/* Init */
void Logger::Init()
{
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

/* Info */
void Logger::Log(const std::string& msg)
{
	LogColor(msg, LOG_COLOR_DEFAULT);
}

/* Success */
void Logger::LogSuccess(const std::string& msg)
{
	LogColor(msg, LOG_COLOR_GREEN);
}

/* Warning */
void Logger::LogWarning(const std::string& msg)
{
	LogColor(msg, LOG_COLOR_YELLOW);
}

/* Error */
void Logger::LogError(const std::string& msg)
{
	LogColor(msg, LOG_COLOR_RED);
}


/*
 * INTERNAL
 */

void Logger::LogColor(const std::string& msg, int color)
{
	SetConsoleTextAttribute(hConsole, color);
	std::cout << msg << std::endl;
	SetConsoleTextAttribute(hConsole, LOG_COLOR_DEFAULT);
}
