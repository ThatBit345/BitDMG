#pragma once
#include <array>

class Memory
{
private:
	std::array<unsigned char, 0xFFFF> memory;

public:

	Memory();

	bool LoadRom(const char* path);

	unsigned char readU8mem(int address);
	void writeU8mem(int address, unsigned char value);

	unsigned short readU16mem(int address);
	void writeU16mem(int address, unsigned short value);
	void writeU16mem(int address, unsigned char lsb, unsigned char msb);
};

