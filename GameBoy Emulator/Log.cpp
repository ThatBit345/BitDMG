#include "Log.h"

#include <iostream>

void Log::LogInfo(const char* txt)
{
	std::cout << "[INFO] " << txt << std::endl;
}

void Log::LogWarning(const char* txt)
{
	std::cout << "[WARNING] " << txt << std::endl;
}

void Log::LogError(const char* txt)
{
	std::cout << "[ERROR] " << txt << std::endl;
}