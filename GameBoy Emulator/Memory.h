#pragma once
#include <array>

#include "Cartridge.h"

class Memory
{
public:
	Memory();

	bool LoadCartridge(Cartridge cart);

	unsigned char ReadU8(int address);
	void WriteU8(int address, unsigned char value);

	unsigned short ReadU16(int address);
	void WriteU16(int address, unsigned short value);
	void WriteU16(int address, unsigned char lsb, unsigned char msb);

private:
	std::array<unsigned char, 0xFFFF> memory;
};

