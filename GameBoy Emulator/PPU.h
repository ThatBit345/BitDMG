#pragma once
#include <memory>

#include "Memory.h"

class PPU
{
public:
	PPU(std::shared_ptr<Memory> memory);

	void Tick(int cycles);

	void PrintTiles();

private:
	std::shared_ptr<Memory> m_Mem;

	int m_Clock;
	int m_Mode;

	void IncrementLY();
};
