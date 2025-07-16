#pragma once

namespace Log
{
	void LogCustom(const char* txt, const char* header);
	void LogInfo(const char* txt);
	void LogWarning(const char* txt);
	void LogError(const char* txt);
}