#include "PPU.h"

#include <iostream>

#include "Log.h"
#include "Utils.h"

PPU::PPU(std::shared_ptr<Memory> memory) : m_Clock(0), m_Mode(0), m_LCD(memory),
										   m_STAT(false), m_LYCSTAT(false), m_Mode2STAT(false), m_Mode1STAT(false), m_Mode0STAT(false)
{
	m_Mem = memory;
}

void PPU::Tick(int cycles)
{
	unsigned char LCDC = m_Mem->ReadU8(IO::LCDC);

	// Disable PPU
	if (GetBit(LCDC, 7) == false)
	{
		m_Mem->UnlockOAM();
		m_Mem->UnlockVRAM();

		// Set STAT to mode 0
		m_Mem->WriteU8Unfiltered(IO::STAT, m_Mem->ReadU8(IO::STAT) & 0b11111100);
		m_LCD.DisableLCD();
		return;
	}

	m_LCD.bgEnabled = LCDC & 0b1;

	m_Clock += cycles;

	// STAT Interrupt handling
	unsigned char STAT = m_Mem->ReadU8(IO::STAT);
	bool statMode2 = GetBit(STAT, 5);
	bool statMode1 = GetBit(STAT, 4);
	bool statMode0 = GetBit(STAT, 3);

	switch (m_Mode)
	{
	// H-Blank
	case 0:
		if (m_Clock >= 204)
		{
			m_Clock -= 204;

			IncrementLY();

			// Switch to V-Blank if we're at the LCD's last scanline
			if (m_Mem->ReadU8(IO::LY) == 143)
			{
				m_Mode = 1;
				m_Mem->WriteU8Unfiltered(IO::STAT, (m_Mem->ReadU8(IO::STAT & 0b11111100) | 0b01)); // Set STAT register flag

				if (statMode1)
				{
					m_Mode1STAT = true;
					HandleSTAT();
				}
				else
				{
					m_Mode1STAT = false;
					HandleSTAT();
				}

				m_Clock++;

				// V-Blank interrupt
				m_Mem->RequestInterrupt(InterruptType::VBLANK);
			}
			else
			{
				m_Mode = 2;
				m_Mem->WriteU8Unfiltered(IO::STAT, (m_Mem->ReadU8(IO::STAT) & 0b11111100) | 0b10); // Set STAT register flag
				m_Mem->LockOAM();

				if (statMode2)
				{
					m_Mode2STAT = true;
					HandleSTAT();
				}
				else
				{
					m_Mode2STAT = false;
					HandleSTAT();
				}
			}
		}
		break;

	// V-Blank
	case 1:
		if (m_Clock % 456 == 0) // One scanline's time
			IncrementLY();

		if (m_Clock >= 4560)
		{
			// Reset clock counter & LY
			m_Clock -= 4560;
			m_Mem->WriteU8(IO::LY, 0);

			m_Mem->LockOAM();

			m_Mode = 2;
			m_Mem->WriteU8Unfiltered(IO::STAT, (m_Mem->ReadU8(IO::STAT) & 0b11111100) | 0b10); // Set STAT register flag

			if (statMode2)
			{
				m_Mode2STAT = true;
				HandleSTAT();
			}
			else
			{
				m_Mode2STAT = false;
				HandleSTAT();
			}
		}
		break;

	// OAM Scan
	case 2:
		m_Mem->LockOAM();

		if (m_Clock >= 80)
		{
			unsigned char LY = m_Mem->ReadU8(IO::LY);

			// Array of sprite addresses to use when drawing the scanline
			std::array<int, 10> spriteArray;
			spriteArray.fill(-1);
			int currentArrayIndex = 0;

			// Get sprites if enabled for this scanline
			if (((LCDC >> 1) & 0b1) == true)
			{
				// If sprites are 16 pixels tall, consider that as the Y offset for the check
				int spriteYOffset = GetBit(LCDC, 2) ? 16 : 8;

				// Select 10 sprites for current scanline
				for (int i = 0; i < 40; i++)
				{
					// New sprite every 4 bytes, byte 0 stores Y position
					// We subtract 16 to eliminate the Gameboy's offset and compare directly with LY
					int spriteY = m_Mem->ReadU8Unfiltered(0xFE00 + (i * 4)) - 16;

					if (LY >= spriteY && LY < spriteY + spriteYOffset)
					{
						spriteArray[currentArrayIndex++] = 0xFE00 + (i * 4);

						if (currentArrayIndex == 10)
							break;
					}
				}
			}
			m_LCD.SetSprites(spriteArray);

			m_Clock -= 80;
			m_Mode = 3;

			m_Mem->WriteU8Unfiltered(IO::STAT, (m_Mem->ReadU8(IO::STAT) & 0b11111100) | 0b11); // Set STAT register flag

			m_Mem->LockVRAM();
		}

		break;

	// Scanline draw
	case 3:
		m_Mem->LockOAM();
		m_Mem->LockVRAM();

		if (m_Clock >= 172)
		{
			m_LCD.DrawScanline(m_Mem->ReadU8(IO::LY));

			m_Clock -= 172;
			m_Mode = 0;

			m_Mem->WriteU8Unfiltered(IO::STAT, m_Mem->ReadU8(IO::STAT) & 0b11111100); // Set STAT register flag

			if (statMode0)
			{
				m_Mode0STAT = true;
				HandleSTAT();
			}
			else
			{
				m_Mode0STAT = false;
				HandleSTAT();
			}

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
			color = (GetBit(msb, j) << 1) | GetBit(lsb, j);

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

void PPU::SetLCDPalette(int id)
{
    m_LCD.SetActivePalette(id);
}

void PPU::Render()
{
	m_LCD.Render();
}

void PPU::HandleSTAT()
{
	// Request interrupt if STAT line was LOW but one of the inputs just changed to HIGH
	bool requestInterrupt = !m_STAT && (m_LYCSTAT || m_Mode2STAT || m_Mode1STAT || m_Mode0STAT);

	if (requestInterrupt)
	{
		m_Mem->RequestInterrupt(InterruptType::STAT_LCD);
	}

	m_STAT = m_LYCSTAT || m_Mode2STAT || m_Mode1STAT || m_Mode0STAT;
}

void PPU::IncrementLY()
{
	unsigned char LY = m_Mem->ReadU8(IO::LY) + 1;
	m_Mem->WriteU8(IO::LY, LY);

	// Compare with LYC
	unsigned char LYC = m_Mem->ReadU8(IO::LYC);
	if (LYC == LY)
	{
		unsigned char STAT = m_Mem->ReadU8(IO::STAT);
		bool statEquals = GetBit(STAT, 6);

		// Set STAT line to high
		if (statEquals)
		{
			m_LYCSTAT = true;
			HandleSTAT();
		}
		m_Mem->WriteU8Unfiltered(IO::STAT, m_Mem->ReadU8(IO::STAT) | 0b100); // Set STAT register flag
	}
	else
	{
		m_LYCSTAT = false;
		HandleSTAT();
		m_Mem->WriteU8Unfiltered(IO::STAT, m_Mem->ReadU8(IO::STAT) & 0b11111011); // Disable STAT register flag
	}
}
