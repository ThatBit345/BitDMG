#include "PPU.h"

#include <iostream>
#include "Log.h"

PPU::PPU(std::shared_ptr<Memory> memory) : m_Clock(0), m_Mode(0)
{
	m_Mem = memory;
}

void PPU::Tick(int cycles)
{
	m_Clock += cycles;

	switch (m_Mode) 
	{
	case 0: // H-Blank
		if(m_Clock >= 204)
		{
			m_Clock = 0;

			IncrementLY();

			// Switch to V-Blank if we're at the LCD's last scanline
			if (m_Mem->ReadU8(0xFF44) == 143) 
			{
				m_Mode = 1;
			}
			else 
			{
				m_Mode = 2;
				m_Mem->LockOAM();
			}
		}
		break;

	case 1: // V-Blank
		if(m_Clock >= 456) // One scanline's time
			IncrementLY();

		if(m_Clock >= 4560)
		{
			// Reset clock counter & LY
			m_Clock = 0;
			m_Mem->WriteU8(0xFF44, 0);

			m_Mem->LockOAM();

			m_Mode = 2;
		}
		break;

	case 2: // OAM Scan
		if(m_Clock >= 80)
		{
			m_Clock = 0;
			m_Mode = 3;

			m_Mem->LockVRAM();
		}

		break;

	case 3: // Scanline draw
		if(m_Clock >= 172)
		{
			// DRAW SCANLINE TO SCREEN

			m_Clock = 0;
			m_Mode = 0;

			m_Mem->UnlockOAM();
			m_Mem->UnlockVRAM();
		}

		break;
	}
}

void PPU::PrintTiles()
{
	for (int i = 0; i < 6143; i++)
	{
		unsigned char lsb = m_Mem->ReadU8Unfiltered(0x8000 + (i * 2));
		unsigned char msb = m_Mem->ReadU8Unfiltered(0x8000 + 1 + (i * 2));

		for (int j = 7; j >= 0; j--)
		{
			unsigned char color = 0;
			color = (((msb >> j) & 0b1) << 1) | ((lsb >> j) & 0b1);

			switch (color)
			{
			case 0b00:
				std::cout << " ";
				break;
			case 0b01:
				std::cout << "░";
				break;
			case 0b10:
				std::cout << "▓";
				break;
			case 0b11:
				std::cout << "█";
				break;
			}
		}
		std::cout << std::endl;
	}

	Log::LogCustom("Finished VRAM tile data dump", "LCD");
}

void PPU::IncrementLY()
{
	unsigned char LY = m_Mem->ReadU8(0xFF44);
	m_Mem->WriteU8(0xFF44, LY + 1);
}
