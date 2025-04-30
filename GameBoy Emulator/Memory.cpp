#include "Memory.h"

#include <string>
#include <iostream>
#include <fstream>

#include "Log.h"

Memory::Memory()
{
	this->memory.fill(0); // Real GB memory would contain random data on start
}

bool Memory::LoadRom(const char* path)
{
	std::string logTxt = "Loading ROM file: " + std::string(path);
	Log::LogInfo(logTxt.c_str());

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

	Log::LogInfo("Loaded ROM succesfully!");

	return true;
}


unsigned char Memory::readU8mem(int address)
{
	return this->memory[address];
}

void Memory::writeU8mem(int address, unsigned char value)
{
	this->memory[address] = value;
}

unsigned short Memory::readU16mem(int address)
{
	unsigned char lsb = this->memory[address];
	unsigned char msb = this->memory[address + 1];

	return ((unsigned short)lsb << 8) | msb;
}

void Memory::writeU16mem(int address, unsigned short value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	this->memory[address] = lsb;
	this->memory[address + 1] = msb;
}

void Memory::writeU16mem(int address, unsigned char lsb, unsigned char msb)
{
	this->memory[address] = lsb;
	this->memory[address + 1] = msb;
}