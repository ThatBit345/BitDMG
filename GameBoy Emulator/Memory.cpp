#include "Memory.h"
#include <iostream>
#include <fstream>

#include "Log.h"

Memory::Memory()
{
	this->memory.fill(0); // Real GB memory would contain random data on start
}

bool Memory::LoadRom(const char* path)
{
	std::ifstream file(path, std::ios::out | std::ios::binary);
	if (!file)
	{
		Log::LogError("Failed to open ROM file!");
		return false;
	}

	file.read((char*)this->memory.data(), 0x7FFF);

	if(!file.good())
	{
		Log::LogError("Error while reading ROM file!");
		return false;
	}

	return true;
}
