#pragma once
#include <array>

class Memory
{
public:
	std::array<unsigned char, 0xFFFF> memory;

	Memory();

	bool LoadRom(const char* path);
};

