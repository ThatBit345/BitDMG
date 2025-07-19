#include "Memory.h"

#include <string>
#include <iostream>
#include <fstream>

#include "Log.h"

Memory::Memory(std::shared_ptr<Cartridge> cart) : m_Cartridge(cart), m_VRAMLocked(false), m_OAMLocked(false)
{
	m_Memory.fill(0);

	// Mimic hardware register's state after boot ROM
	m_Memory[0xFF00] = 0xCF; //P1
	m_Memory[0xFF01] = 0x00; //SB
	m_Memory[0xFF02] = 0x7E; //SC
	m_Memory[0xFF04] = 0x18; //DIV
	m_Memory[0xFF05] = 0x00; //TIMA
	m_Memory[0xFF06] = 0x00; //TMA
	m_Memory[0xFF07] = 0xF8; //TAC
	m_Memory[0xFF0F] = 0xE1; //IF
	m_Memory[0xFF10] = 0x80; //NR10
	m_Memory[0xFF11] = 0xBF; //NR11
	m_Memory[0xFF12] = 0xF3; //NR12
	m_Memory[0xFF13] = 0xFF; //NR13
	m_Memory[0xFF14] = 0xBF; //NR14
	m_Memory[0xFF16] = 0x3F; //NR21
	m_Memory[0xFF17] = 0x00; //NR22
	m_Memory[0xFF18] = 0xFF; //NR23
	m_Memory[0xFF19] = 0xBF; //NR24
	m_Memory[0xFF1A] = 0x7F; //NR30
	m_Memory[0xFF1B] = 0xFF; //NR31
	m_Memory[0xFF1C] = 0x9F; //NR32
	m_Memory[0xFF1D] = 0xFF; //NR33
	m_Memory[0xFF1E] = 0xBF; //NR34
	m_Memory[0xFF20] = 0xFF; //NR41
	m_Memory[0xFF21] = 0x00; //NR42
	m_Memory[0xFF22] = 0x00; //NR43
	m_Memory[0xFF23] = 0xBF; //NR44
	m_Memory[0xFF24] = 0x77; //NR50
	m_Memory[0xFF25] = 0xF3; //NR51
	m_Memory[0xFF26] = 0xF1; //NR52
	m_Memory[0xFF40] = 0x91; //LCDC
	m_Memory[0xFF41] = 0x81; //STAT
	m_Memory[0xFF42] = 0x00; //SCY
	m_Memory[0xFF43] = 0x00; //SCX
	m_Memory[0xFF44] = 0x91; //LY
	m_Memory[0xFF45] = 0x00; //LYC
	m_Memory[0xFF46] = 0xFF; //DMA
	m_Memory[0xFF47] = 0xFC; //BGP
	m_Memory[0xFF4A] = 0x00; //WY
	m_Memory[0xFF4B] = 0x00; //WX
	m_Memory[0xFFFF] = 0x00; //IE
}

/* Get 8-bit value.
*  [address] -> Memory address to read
*/
unsigned char Memory::ReadU8(unsigned short address)
{
	if (address <= 0x7FFF)
	{
		return m_Cartridge->ReadU8(address);
	}
	else if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked) return 0xFF; // VRAM Locked
	else if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked) return 0xFF; // OAM Locked
	else if (address >= 0xFEA0 && address <= 0xFEFF) return 0xFF; // Prohibited area, returns 0xFF if OAM is blocked, 0x00 otherwise (triggers OAM corruption)
	else if (address == 0xFF00) return 0xF;
	return m_Memory[address];
}

/* Get 8-bit value without considering Gameboy state.
*  [address] -> Memory address to read
*/
unsigned char Memory::ReadU8Unfiltered(unsigned short address)
{
	return m_Memory[address];
}

/* Write 8-bit value.
*  [address] -> Memory address to write
*  [value] -> Value to write
*/
void Memory::WriteU8(unsigned short address, unsigned char value)
{
	// If writting in rom check for mapper registers
	if (address <= 0x7FFF) m_Cartridge->CheckROMWrite(address, value);
	//else if (address == 0xFF01) Log::LogCustom((char*)&value, "SERIAL OUT"); // Trap serial output and log it
	else if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address] = value;
		m_Memory[address + 0x2000] = value; // Write to echo RAM
	}
	else if (address == 0xFF04) m_Memory[0xFF04] = 0x00; // Trap timer's DIV register
	else if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked) return; // VRAM Locked
	else if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked) return; // OAM Locked
	else if (address == 0xFF46) // DMA Transfer
	{
		unsigned short source = value * 0x100;

		for (size_t i = 0; i < 159; i++)
		{
			m_Memory[0xFE00 + i] = m_Memory[source + i];
		}
	}
	else m_Memory[address] = value;
}

/* Write 8-bit value without considering Gameboy state.
*  [address] -> Memory address to write
*  [value] -> Value to write
*/
void Memory::WriteU8Unfiltered(unsigned short address, unsigned char value)
{
	m_Memory[address] = value; 
	
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = value; // Write to echo RAM
	}
	else if (address == 0xFF46) // DMA Transfer
	{
		unsigned short source = value * 0x100;

		for (size_t i = 0; i < 159; i++)
		{
			m_Memory[0xFE00 + i] = m_Memory[source + i];
		}
	}
}

/* Get 16-bit value.
*  [address] -> Memory address to read
*/
unsigned short Memory::ReadU16(unsigned short address)
{
	if (address <= 0x7FFF)
	{
		return m_Cartridge->ReadU16(address);
	}
	else if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked) return 0xFFFF; // VRAM Locked
	else if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked) return 0xFFFF; // OAM Locked
	else if (address >= 0xFEA0 && address <= 0xFEFF) return 0xFFFF; // Prohibited area, returns 0xFF if OAM is blocked, 0x00 otherwise (triggers OAM corruption)

	unsigned char lsb = m_Memory[address];
	unsigned char msb = m_Memory[address + 1];

	return ((unsigned short)msb << 8) | lsb;
}

/* Write 16-bit value.
*  [address] -> Memory address to write
*  [value] -> Value to write
*/
void Memory::WriteU16(unsigned short address, unsigned short value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	// No writting in ROM
	if (address <= 0x7FFF) return;
	else if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked) return; // VRAM Locked
	else if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked) return; // OAM Locked
	else if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address] = lsb;
		m_Memory[address + 1] = msb;
		m_Memory[address + 0x2000] = lsb; // Write to echo RAM
		m_Memory[address + 0x2001] = msb; // Write to echo RAM
	}
	else 
	{
		m_Memory[address] = lsb;
		m_Memory[address + 1] = msb;
	}
}

/* Write 8-bit value.
*  [address] -> Memory address to write
*  [lsb] -> Least significant bit of the value to write
*  [msb] -> Most significant bit of the value to write
*/
void Memory::WriteU16(unsigned short address, unsigned char lsb, unsigned char msb)
{
	// No writting in ROM
	if (address <= 0x7FFF) return;
	else if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked) return; // VRAM Locked
	else if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked) return; // OAM Locked
	else if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address] = lsb;
		m_Memory[address + 1] = msb;
		m_Memory[address + 0x2000] = lsb; // Write to echo RAM
		m_Memory[address + 0x2001] = msb; // Write to echo RAM
	}
	else {
		m_Memory[address] = lsb;
		m_Memory[address + 1] = msb;
	}
}

/* Write 16-bit value without considering Gameboy state.
*  [address] -> Memory address to write
*  [value] -> Value to write
*/
void Memory::WriteU16Unfiltered(unsigned short address, unsigned char value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	m_Memory[address] = lsb;
	m_Memory[address + 1] = msb; 
	
	if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address + 0x2000] = lsb; // Write to echo RAM
		m_Memory[address + 0x2001] = msb; // Write to echo RAM
	}
}

/* Write 16-bit value to the stack.
*  [address] -> Memory address to write
*  [value] -> Value to write
*/
void Memory::WriteU16Stack(unsigned short address, unsigned short value)
{
	unsigned char lsb = (unsigned char)value;
	unsigned char msb = (unsigned char)(value >> 8);

	// No writting in ROM
	if (address <= 0x7FFF) return;
	else if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked) return; // VRAM Locked
	else if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked) return; // OAM Locked
	else if (address >= 0xC000 && address <= 0xDDFF)
	{
		m_Memory[address] = msb;
		m_Memory[address - 1] = lsb;
		m_Memory[address + 0x2000] = msb; // Write to echo RAM
		m_Memory[address - 0x2001] = lsb; // Write to echo RAM
	}
	else {
		m_Memory[address] = msb;
		m_Memory[address - 1] = lsb;
	}
}

void Memory::LockVRAM()
{
	m_VRAMLocked = true;
}

void Memory::UnlockVRAM()
{
	m_VRAMLocked = false;
}

void Memory::LockOAM()
{
	m_OAMLocked = true;
}

void Memory::UnlockOAM()
{
	m_OAMLocked = false;
}