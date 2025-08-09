#include "Memory.h"

#include <string>
#include <iostream>
#include <fstream>

#include "Log.h"
#include "Utils.h"

Memory::Memory(std::shared_ptr<Cartridge> cart) : m_Cartridge(cart), m_VramLocked(false), m_OamLocked(false)
{
	m_Memory.fill(0);

	// Mimic hardware register's state after boot ROM
	m_Memory[IO::JOY] = 0xCF;
	m_Memory[IO::SB] = 0x00; 
	m_Memory[IO::SC] = 0x7E; 
	m_Memory[IO::DIV] = 0x18;
	m_Memory[IO::TIMA] = 0x00; 
	m_Memory[IO::TMA] = 0x00; 
	m_Memory[IO::TAC] = 0xF8; 
	m_Memory[IO::IF] = 0xE1;
	m_Memory[IO::NR10] = 0x80; 
	m_Memory[IO::NR11] = 0xBF; 
	m_Memory[IO::NR12] = 0xF3; 
	m_Memory[IO::NR13] = 0xFF; 
	m_Memory[IO::NR14] = 0xBF; 
	m_Memory[IO::NR21] = 0x3F; 
	m_Memory[IO::NR22] = 0x00; 
	m_Memory[IO::NR23] = 0xFF; 
	m_Memory[IO::NR24] = 0xBF; 
	m_Memory[IO::NR30] = 0x7F; 
	m_Memory[IO::NR31] = 0xFF; 
	m_Memory[IO::NR32] = 0x9F; 
	m_Memory[IO::NR33] = 0xFF; 
	m_Memory[IO::NR34] = 0xBF; 
	m_Memory[IO::NR41] = 0xFF; 
	m_Memory[IO::NR42] = 0x00; 
	m_Memory[IO::NR43] = 0x00; 
	m_Memory[IO::NR44] = 0xBF; 
	m_Memory[IO::NR50] = 0x77; 
	m_Memory[IO::NR51] = 0xF3; 
	m_Memory[IO::NR52] = 0xF1; 
	m_Memory[IO::LCDC] = 0x91; 
	m_Memory[IO::STAT] = 0x81; 
	m_Memory[IO::SCY] = 0x00; 
	m_Memory[IO::SCX] = 0x00; 
	m_Memory[IO::LY] = 0x91; 
	m_Memory[IO::LYC] = 0x00; 
	m_Memory[IO::DMA] = 0xFF; 
	m_Memory[IO::BGP] = 0xFC; 
	m_Memory[IO::WY] = 0x00; 
	m_Memory[IO::WX] = 0x00; 
	m_Memory[IO::IE] = 0x00; 
}

unsigned char Memory::ReadU8(unsigned short address)
{
	// Cartridge ROM
	if (address <= 0x7FFF)
	{
		return m_Cartridge->ReadU8(address);
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VramLocked)
	{
		return 0xFF;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		return m_Cartridge->ReadU8RAM(address);
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OamLocked)
	{
		return 0xFF;
	}

	// Prohibited area
	if (address >= 0xFEA0 && address <= 0xFEFF)
	{
		return m_OamLocked ? 0xFF : 0x00;
	}

	// Joypad register
	if (address == IO::JOY)
	{
		UpdateInputRegister();
		return m_Memory[address];
	}

	// Serial
	if (address == IO::SB)
	{
		return 0xFF;
	}

	return m_Memory[address];
}

unsigned char Memory::ReadU8Unfiltered(unsigned short address)
{
	// Cartridge ROM
	if (address <= 0x7FFF)
	{
		return m_Cartridge->ReadU8(address);
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		return m_Cartridge->ReadU8RAM(address);
	}

	return m_Memory[address];
}

void Memory::WriteU8(unsigned short address, unsigned char value)
{
	// Cartridge ROM -> Update mapper registers
	if (address <= 0x7FFF)
	{
		m_Cartridge->CheckROMWrite(address, value);
		return;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		m_Cartridge->WriteU8RAM(address, value);
		return;
	}

	// Trap serial output and log it
	// if (address == 0xFF01) Log::LogCustom((char*)&value, "SERIAL OUT");

	// Trap the timer's DIV register
	if (address == IO::DIV)
	{
		m_Memory[IO::DIV] = 0x00;
		return;
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VramLocked)
	{
		return;
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OamLocked)
	{
		return;
	}

	// DMA Transfer
	if (address == IO::DMA)
	{
		unsigned short source = value * 0x100;

		for (size_t i = 0; i < 159; i++)
		{
			m_Memory[0xFE00 + i] = m_Memory[source + i];
		}

		return;
	}

	m_Memory[address] = value;

	// Internal RAM, when writting to this area the changes are replicated at Echo RAM
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = value;
	}
}

void Memory::WriteU8Unfiltered(unsigned short address, unsigned char value)
{
	// Cartridge ROM -> Update mapper registers
	if (address <= 0x7FFF)
	{
		m_Cartridge->CheckROMWrite(address, value);
		return;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		m_Cartridge->WriteU8RAM(address, value);
		return;
	}

	// DMA Transfer
	if (address == IO::DMA)
	{
		unsigned short source = value * 0x100;

		for (size_t i = 0; i < 159; i++)
		{
			m_Memory[0xFE00 + i] = m_Memory[source + i];
		}

		return;
	}

	m_Memory[address] = value;

	// Internal RAM, when writting to this area the changes are replicated at Echo RAM
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = value;
	}
}

unsigned short Memory::ReadU16(unsigned short address)
{
	// Cartridge ROM
	if (address <= 0x7FFF)
	{
		return m_Cartridge->ReadU16(address);
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VramLocked)
	{
		return 0xFFFF;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		return m_Cartridge->ReadU16RAM(address);
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OamLocked)
	{
		return 0xFFFF;
	}

	// Prohibited area
	if (address >= 0xFEA0 && address <= 0xFEFF)
	{
		return m_OamLocked ? 0x00FF : 0x0000;
	}

	unsigned char lsb = m_Memory[address];
	unsigned char msb = m_Memory[address + 1];

	return ((unsigned short)msb << 8) | lsb;
}

void Memory::WriteU16(unsigned short address, unsigned short value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	// Cartridge ROM, forbidden
	if (address <= 0x7FFF)
	{
		return;
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VramLocked)
	{
		return;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		m_Cartridge->WriteU16RAM(address, value);
		return;
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OamLocked)
	{
		return;
	}

	m_Memory[address] = lsb;
	m_Memory[address + 1] = msb;

	// If we wrote to internal RAM, replicate changes to Echo RAM
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = lsb; // Write to echo RAM
		m_Memory[address + 0x2001] = msb; // Write to echo RAM
	}
}

void Memory::WriteU16(unsigned short address, unsigned char lsb, unsigned char msb)
{
	// Cartridge ROM, forbidden
	if (address <= 0x7FFF)
	{
		return;
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VramLocked)
	{
		return;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		m_Cartridge->WriteU16RAM(address, lsb, msb);
		return;
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OamLocked)
	{
		return;
	}

	m_Memory[address] = lsb;
	m_Memory[address + 1] = msb;

	// If we wrote to internal RAM, replicate changes to Echo RAM
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = lsb; // Write to echo RAM
		m_Memory[address + 0x2001] = msb; // Write to echo RAM
	}
}

void Memory::WriteU16Unfiltered(unsigned short address, unsigned char value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		m_Cartridge->WriteU16RAM(address, value);
		return;
	}

	m_Memory[address] = lsb;
	m_Memory[address + 1] = msb;

	// If we wrote to internal RAM, replicate changes to Echo RAM
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = lsb; // Write to echo RAM
		m_Memory[address + 0x2001] = msb; // Write to echo RAM
		return;
	}
}

void Memory::WriteU16Stack(unsigned short address, unsigned short value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	// Cartridge ROM, forbidden
	if (address <= 0x7FFF)
	{
		return;
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VramLocked)
	{
		return;
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OamLocked)
	{
		return;
	}

	m_Memory[address] = msb;
	m_Memory[address - 1] = lsb;

	// If we wrote to internal RAM, replicate changes to Echo RAM
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = msb;
		m_Memory[address - 0x2001] = lsb;
	}
}

void Memory::LockVRAM()
{
	m_VramLocked = true;
}

void Memory::UnlockVRAM()
{
	m_VramLocked = false;
}

void Memory::LockOAM()
{
	m_OamLocked = true;
}

void Memory::UnlockOAM()
{
	m_OamLocked = false;
}

void Memory::RequestInterrupt(InterruptType interrupt)
{
	WriteU8Unfiltered(IO::IF, ReadU8(IO::IF) | (1 << (int)interrupt));
}

void Memory::UpdateInputState(bool buffer[8])
{
	for (size_t i = 0; i < 8; i++)
	{
		m_InputBuffer[i] = buffer[i];
	}
}

void Memory::UpdateInputRegister()
{
	unsigned char P1 = m_Memory[IO::JOY];
	bool joypad5 = GetBit(P1, 5);
	bool joypad4 = GetBit(P1, 4);

	unsigned char input = 0;

	// 4 Low & 5 High -> D-Pad
	if (!joypad4 && joypad5) 
	{
		HandleInputInterrupt(false);

		input = 0x20 + (!m_InputBuffer[InputButtons::DOWN] << 3) + (!m_InputBuffer[InputButtons::UP] << 2) + (!m_InputBuffer[InputButtons::LEFT] << 1) + (!m_InputBuffer[InputButtons::RIGHT] << 0);
	}
	// 4 High & 5 Low -> Buttons
	else if (joypad4 && !joypad5) 
	{
		HandleInputInterrupt(true);

		input = 0x10 + (!m_InputBuffer[InputButtons::START] << 3) + (!m_InputBuffer[InputButtons::SELECT] << 2) + (!m_InputBuffer[InputButtons::B] << 1) + (!m_InputBuffer[InputButtons::A] << 0);
	}
	else
	{
		input = 0x3F;
	}

	m_Memory[IO::JOY] = input;
}

void Memory::HandleInputInterrupt(bool isButtons)
{
	unsigned char P1 = m_Memory[IO::JOY];

	bool inputBits[4];
	for (size_t i = 0; i < 4; i++)
	{
		inputBits[i] = ~(GetBit(P1, i));
	}

	for (size_t i = 0; i < 4; i++)
	{
		if (!inputBits[i] && m_InputBuffer[i - (isButtons * 4)])
		{
			RequestInterrupt(InterruptType::JOYPAD);
		}
	}
}
