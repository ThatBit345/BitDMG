#include "Memory.h"

#include <string>
#include <iostream>
#include <fstream>

#include "Log.h"
#include "Utils.h"

Memory::Memory(std::shared_ptr<Cartridge> cart) : m_Cartridge(cart), m_VRAMLocked(false), m_OAMLocked(false)
{
	m_Memory.fill(0);

	// Mimic hardware register's state after boot ROM
	m_Memory[0xFF00] = 0xCF; // P1
	m_Memory[0xFF01] = 0x00; // SB
	m_Memory[0xFF02] = 0x7E; // SC
	m_Memory[0xFF04] = 0x18; // DIV
	m_Memory[0xFF05] = 0x00; // TIMA
	m_Memory[0xFF06] = 0x00; // TMA
	m_Memory[0xFF07] = 0xF8; // TAC
	m_Memory[0xFF0F] = 0xE1; // IF
	m_Memory[0xFF10] = 0x80; // NR10
	m_Memory[0xFF11] = 0xBF; // NR11
	m_Memory[0xFF12] = 0xF3; // NR12
	m_Memory[0xFF13] = 0xFF; // NR13
	m_Memory[0xFF14] = 0xBF; // NR14
	m_Memory[0xFF16] = 0x3F; // NR21
	m_Memory[0xFF17] = 0x00; // NR22
	m_Memory[0xFF18] = 0xFF; // NR23
	m_Memory[0xFF19] = 0xBF; // NR24
	m_Memory[0xFF1A] = 0x7F; // NR30
	m_Memory[0xFF1B] = 0xFF; // NR31
	m_Memory[0xFF1C] = 0x9F; // NR32
	m_Memory[0xFF1D] = 0xFF; // NR33
	m_Memory[0xFF1E] = 0xBF; // NR34
	m_Memory[0xFF20] = 0xFF; // NR41
	m_Memory[0xFF21] = 0x00; // NR42
	m_Memory[0xFF22] = 0x00; // NR43
	m_Memory[0xFF23] = 0xBF; // NR44
	m_Memory[0xFF24] = 0x77; // NR50
	m_Memory[0xFF25] = 0xF3; // NR51
	m_Memory[0xFF26] = 0xF1; // NR52
	m_Memory[0xFF40] = 0x91; // LCDC
	m_Memory[0xFF41] = 0x81; // STAT
	m_Memory[0xFF42] = 0x00; // SCY
	m_Memory[0xFF43] = 0x00; // SCX
	m_Memory[0xFF44] = 0x91; // LY
	m_Memory[0xFF45] = 0x00; // LYC
	m_Memory[0xFF46] = 0xFF; // DMA
	m_Memory[0xFF47] = 0xFC; // BGP
	m_Memory[0xFF4A] = 0x00; // WY
	m_Memory[0xFF4B] = 0x00; // WX
	m_Memory[0xFFFF] = 0x00; // IE
}

/* Get 8-bit value.
 *  [address] -> Memory address to read
 */
unsigned char Memory::ReadU8(unsigned short address)
{
	// Cartridge ROM
	if (address <= 0x7FFF)
	{
		return m_Cartridge->ReadU8(address);
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked)
	{
		return 0xFF;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		return m_Cartridge->ReadU8RAM(address);
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked)
	{
		return 0xFF;
	}

	// Prohibited area
	if (address >= 0xFEA0 && address <= 0xFEFF)
	{
		return m_OAMLocked ? 0xFF : 0x00;
	}

	// Joypad register
	if (address == 0xFF00)
	{
		UpdateInputRegister();
		return m_Memory[address];
	}

	// Serial
	if (address == 0xFF01)
	{
		return 0xFF;
	}

	return m_Memory[address];
}

/* Get 8-bit value without considering Gameboy state.
 *  [address] -> Memory address to read
 */
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

/* Write 8-bit value.
 *  [address] -> Memory address to write
 *  [value] -> Value to write
 */
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
	if (address == 0xFF04)
	{
		m_Memory[0xFF04] = 0x00;
		return;
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked)
	{
		return;
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked)
	{
		return;
	}

	// DMA Transfer
	if (address == 0xFF46)
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

/* Write 8-bit value without considering Gameboy state.
 *  [address] -> Memory address to write
 *  [value] -> Value to write
 */
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
	if (address == 0xFF46)
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

/* Get 16-bit value.
 *  [address] -> Memory address to read
 */
unsigned short Memory::ReadU16(unsigned short address)
{
	// Cartridge ROM
	if (address <= 0x7FFF)
	{
		return m_Cartridge->ReadU16(address);
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked)
	{
		return 0xFFFF;
	}

	// External RAM
	if (address >= 0xA000 && address <= 0xBFFF)
	{
		return m_Cartridge->ReadU16RAM(address);
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked)
	{
		return 0xFFFF;
	}

	// Prohibited area
	if (address >= 0xFEA0 && address <= 0xFEFF)
	{
		return m_OAMLocked ? 0x00FF : 0x0000;
	}

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

	// Cartridge ROM, forbidden
	if (address <= 0x7FFF)
	{
		return;
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked)
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
	if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked)
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

/* Write 8-bit value.
 *  [address] -> Memory address to write
 *  [lsb] -> Least significant bit of the value to write
 *  [msb] -> Most significant bit of the value to write
 */
void Memory::WriteU16(unsigned short address, unsigned char lsb, unsigned char msb)
{
	// Cartridge ROM, forbidden
	if (address <= 0x7FFF)
	{
		return;
	}

	// VRAM Lock
	if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked)
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
	if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked)
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

/* Write 16-bit value without considering Gameboy state.
 *  [address] -> Memory address to write
 *  [value] -> Value to write
 */
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

/* Write 16-bit value to the stack.
 *  [address] -> Memory address to write
 *  [value] -> Value to write
 */
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
	if (address >= 0x8000 && address <= 0x9FFF && m_VRAMLocked)
	{
		return;
	}

	// OAM Lock
	if (address >= 0xFE00 && address <= 0xFE9F && m_OAMLocked)
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

void Memory::UpdateInputState(bool buffer[8])
{
	for (size_t i = 0; i < 8; i++)
	{
		m_InputBuffer[i] = buffer[i];
	}
}

/* Update the joypad register ($FF00 - P1) with data from m_InputBuffer
 */
void Memory::UpdateInputRegister()
{
	unsigned char P1 = m_Memory[0xFF00];
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

	m_Memory[0xFF00] = input;
}

void Memory::HandleInputInterrupt(bool isButtons)
{
	unsigned char P1 = m_Memory[0xFF00];

	bool inputBits[4];
	for (size_t i = 0; i < 4; i++)
	{
		inputBits[i] = ~(GetBit(P1, i));
	}

	for (size_t i = 0; i < 4; i++)
	{
		if (!inputBits[i] && m_InputBuffer[i - (isButtons * 4)])
		{
			m_Memory[0xFF0F] = m_Memory[0xFF0F] | 0b10000;
		}
	}
}
