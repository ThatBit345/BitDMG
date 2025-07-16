#include "PPU.h"

#include <iostream>
#include "Log.h"

PPU::PPU(std::shared_ptr<Memory> memory) : m_Clock(0), m_Mode(0), m_LCD(memory), 
	m_STAT(false), m_LYCSTAT(false), m_Mode2STAT(false), m_Mode1STAT(false), m_Mode0STAT(false)
{
	m_Mem = memory;
}

void PPU::Tick(int cycles)
{
	unsigned char LCDC = m_Mem->ReadU8(0xFF40);

	// Disable PPU
	if(LCDC >> 7 == 0)
	{
		m_Mem->UnlockOAM();
		m_Mem->UnlockVRAM();
	
		//m_LCD.DisableLCD();
		return;
	}

	m_LCD.bgEnabled = LCDC & 0b1;

	m_Clock += cycles;

	// STAT Interrupt handling
	unsigned char STAT = m_Mem->ReadU8(0xFF41);
	bool statMode2 = (STAT & 0b00100000) >> 5;
	bool statMode1 = (STAT & 0b00010000) >> 4;
	bool statMode0 = (STAT & 0b00001000) >> 3;

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
				m_Mem->WriteU8Unfiltered(0xFF41, m_Mem->ReadU8(0xFF41) | 0b01); // Set STAT register flag

				if(statMode1)
				{
					m_Mode1STAT = true;
					HandleSTAT();
				}
				else m_Mode1STAT = false;
				
				// V-Blank interrupt
				m_Mem->WriteU8Unfiltered(0xFF0F, m_Mem->ReadU8(0xFF0F) | 0b1);
			}
			else 
			{
				m_Mode = 2;
				m_Mem->WriteU8Unfiltered(0xFF41, m_Mem->ReadU8(0xFF41) | 0b10); // Set STAT register flag
				m_Mem->LockOAM();

				if(statMode2)
				{
					m_Mode2STAT = true;
					HandleSTAT();
				}
				else m_Mode2STAT = false;
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
			m_Mem->WriteU8Unfiltered(0xFF41, m_Mem->ReadU8(0xFF41) | 0b10); // Set STAT register flag

			if(statMode2)
			{
				m_Mode2STAT = true;
				HandleSTAT();
			}
			else m_Mode2STAT = false;
		}
		break;

	case 2: // OAM Scan
		m_Mem->LockOAM();

		if(m_Clock >= 80)
		{
			m_Clock = 0;
			m_Mode = 3;

			m_Mem->WriteU8Unfiltered(0xFF41, m_Mem->ReadU8(0xFF41) | 0b11); // Set STAT register flag

			m_Mem->LockVRAM();
		}

		break;

	case 3: // Scanline draw
		m_Mem->LockOAM();
		m_Mem->LockVRAM();

		if(m_Clock >= 172)
		{
			m_LCD.DrawScanline(m_Mem->ReadU8(0xFF44));

			m_Clock = 0;
			m_Mode = 0;
			
			m_Mem->WriteU8Unfiltered(0xFF41, m_Mem->ReadU8(0xFF41) | 0b00); // Set STAT register flag

			if(statMode0)
			{
				m_Mode0STAT = true;
				HandleSTAT();
			}
			else m_Mode0STAT = false;

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

	Log::LogCustom("Finished VRAM tile data dump", "PPU");
}

void PPU::ConfigureLCD(SDL_Window *window)
{
	m_LCD.SetWindow(window);
}

void PPU::Render()
{
	m_LCD.Render();
}

void PPU::HandleSTAT()
{
	// Request interrupt if STAT line was LOW but one of the inputs just changed to HIGH
	bool requestInterrupt = !m_STAT && (m_LYCSTAT || m_Mode2STAT || m_Mode1STAT || m_Mode0STAT);

	if(requestInterrupt) 
	{
		m_Mem->WriteU8Unfiltered(0xFF0F, m_Mem->ReadU8(0xFF0F) | 0b10);
	}

	m_STAT = m_LYCSTAT || m_Mode2STAT || m_Mode1STAT || m_Mode0STAT;
}

void PPU::IncrementLY()
{
	unsigned char LY = m_Mem->ReadU8(0xFF44) + 1;
	m_Mem->WriteU8(0xFF44, LY);

	// Compare with LYC
	unsigned char LYC = m_Mem->ReadU8(0xFF45);
	if(LYC == LY)
	{
		unsigned char STAT = m_Mem->ReadU8(0xFF41);
		bool statEquals = (STAT & 0b01000000) >> 6;
		
		// Set STAT line to high
		if(statEquals) 
		{
			m_LYCSTAT = true;
			HandleSTAT();
		}
		m_Mem->WriteU8Unfiltered(0xFF41, m_Mem->ReadU8(0xFF41) | 0b100); // Set STAT register flag
	}
	else 
	{
		m_LYCSTAT = false;
		m_Mem->WriteU8Unfiltered(0xFF41, m_Mem->ReadU8(0xFF41) & 0b11111011); // Disable STAT register flag
	}
}
