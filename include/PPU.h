#pragma once
#include <memory>

#include "Memory.h"
#include "LCD.h"

class PPU
{
public:
	PPU(std::shared_ptr<Memory> memory);

	void Tick(int cycles);
	void PrintTiles();
	void Render();

	void ConfigureLCD(SDL_Window* window);
	
private:
	std::shared_ptr<Memory> m_Mem;

	LCD m_LCD;

	int m_Clock;
	int m_Mode;

	bool m_STAT;
	bool m_LYCSTAT;
	bool m_Mode2STAT;
	bool m_Mode1STAT;
	bool m_Mode0STAT;

	void IncrementLY();
	void HandleSTAT();
};
