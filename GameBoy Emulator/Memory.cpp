#include "Memory.h"

#include <string>
#include <iostream>
#include <fstream>

#include "Log.h"

Memory::Memory()
{
	this->memory.fill(0); // Real GB memory would contain random data on start
}

bool Memory::LoadCartridge(Cartridge cart)
{
	memcpy(memory.data(), cart.GetRom(), 0x7FFF);

	return true;
}

unsigned char Memory::ReadU8(int address)
{
	return memory[address];
}

void Memory::WriteU8(int address, unsigned char value)
{
	memory[address] = value;
}

unsigned short Memory::ReadU16(int address)
{
	unsigned char lsb = memory[address];
	unsigned char msb = memory[address + 1];

	return ((unsigned short)lsb << 8) | msb;
}

void Memory::WriteU16(int address, unsigned short value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	memory[address] = lsb;
	memory[address + 1] = msb;
}

void Memory::WriteU16(int address, unsigned char lsb, unsigned char msb)
{
	memory[address] = lsb;
	memory[address + 1] = msb;
}