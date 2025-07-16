#include "Log.h"

#include <iostream>

void Log::LogCustom(const char* txt, const char* header)
{
	std::cout << "[" << header << "]" << txt << std::endl;
}

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