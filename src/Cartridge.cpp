#include "Cartridge.h"

#include <string>
#include <iostream>
#include <fstream>

#include <array>

#include "Log.h"
#include <cmath>

Cartridge::Cartridge(const char* romPath) : m_ROMBank(1)
{
	std::string logTxt = "Loading ROM file: " + std::string(romPath);
	Log::LogInfo(logTxt.c_str());

	std::ifstream file(romPath, std::ios::out | std::ios::binary);
	if (!file)
	{
		Log::LogError("Failed to open ROM file!");
		this->m_IsValid = false;
	}

	// Load header and extract metadata
	std::array<unsigned char, 0x014F> header{};
	file.read((char*)header.data(), 0x014F);

	// Get the rom's name
	char name[16];
	for (size_t i = 0; i < 16; i++)
	{
		name[i] = header[0x0134 + i];
	}
	this->m_CartName = std::string(name);

	Log::LogInfo(this->m_CartName.c_str());

	// Get the hardware information
	unsigned char typeByte = header[0x0147];
	switch (typeByte)
	{
	case 0x00: // ROM
		SetHardware(Mapper::None, false, false, false, false, false);
		break;

	case 0x01: // MBC1
		SetHardware(Mapper::MBC1, false, false, false, false, false);
		break;

	case 0x02: // MBC1 + RAM
		SetHardware(Mapper::MBC1, true, false, false, false, false);
		break;

	case 0x03: // MBC1 + RAM + BATTERY
		SetHardware(Mapper::MBC1, true, true, false, false, false);
		break;

	case 0x05: // MBC2
		Log::LogError("Mapper not implemented (MBC2)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC2, false, false, false, false, false);
		break;

	case 0x06: // MBC2 + BATTERY
		Log::LogError("Mapper not implemented (MBC2)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC2, false, true, false, false, false);
		break;

	case 0x08: // ROM + RAM
		SetHardware(Mapper::None, true, false, false, false, false);
		break;

	case 0x09: // ROM + RAM + BATTERY
		SetHardware(Mapper::None, true, true, false, false, false);
		break;

	case 0x0B: // MMM01
		Log::LogError("Mapper not implemented (MMM01)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MMM01, false, false, false, false, false);
		break;

	case 0x0C: // MMM01 + RAM
		Log::LogError("Mapper not implemented (MMM01)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MMM01, true, false, false, false, false);
		break;

	case 0x0D: // MMM01 + RAM + BATTERY
		Log::LogError("Mapper not implemented (MMM01)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MMM01, true, true, false, false, false);
		break;

	case 0x0F: // MBC3 + BATTERY + TIMER
		Log::LogError("Mapper not implemented (MBC3)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC3, false, true, true, false, false);
		break;

	case 0x10: // MBC3 + RAM + BATTERY + TIMER
		Log::LogError("Mapper not implemented (MBC3)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC3, true, true, true, false, false);
		break;

	case 0x11: // MBC3
		Log::LogError("Mapper not implemented (MBC3)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC3, false, false, false, false, false);
		break;

	case 0x12: // MBC3 + RAM
		Log::LogError("Mapper not implemented (MBC3)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC3, true, false, false, false, false);
		break;

	case 0x13: // MBC3 + RAM + BATTERY
		Log::LogError("Mapper not implemented (MBC3)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC3, true, true, false, false, false);
		break;

	case 0x19: // MBC5
		Log::LogError("Mapper not implemented (MBC5)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC5, false, false, false, false, false);
		break;

	case 0x1A: // MBC5 + RAM
		Log::LogError("Mapper not implemented (MBC5)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC5, true, false, false, false, false);
		break;

	case 0x1B: // MBC5 + RAM + BATTERY
		Log::LogError("Mapper not implemented (MBC5)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC5, true, true, false, false, false);
		break;

	case 0x1C: // MBC5 + RUMBLE
		Log::LogError("Mapper not implemented (MBC5)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC5, false, false, false, true, false);
		break;

	case 0x1D: // MBC5 + RAM + RUMBLE
		Log::LogError("Mapper not implemented (MBC5)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC5, true, false, false, true, false);
		break;

	case 0x1E: // MBC5 + RAM + BATTERY + RUMBLE
		Log::LogError("Mapper not implemented (MBC5)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC5, true, true, false, true, false);
		break;

	case 0x20: // MBC6
		Log::LogError("Mapper not implemented (MBC6)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC6, false, false, false, false, false);
		break;

	case 0x22: // MBC7 + RAM + BATTERY + RUMBLE + SENSOR
		Log::LogError("Mapper not implemented (MBC7)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::MBC7, true, true, false, true, true);
		break;

	case 0xFE: // HuC3
		Log::LogError("Mapper not implemented (HuC3)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::HuC3, false, false, false, false, false);
		break;

	case 0xFF: // HuC1 + RAM + BATTERY
		Log::LogError("Mapper not implemented (HuC1)");
		m_IsValid = false;
		return;

		SetHardware(Mapper::HuC1, true, true, false, false, false);
		break;
	}

	// Calculate the size of the rom specified in the header
	size_t romSize = 32768 * std::pow(2, header[0x0148]);
	this->m_Rom.resize(romSize);

	// Copy ROM into cartridge memory
	file.seekg(0, std::ios::beg);

	if(!file.good())
	{
		Log::LogError("Error while reading ROM file!");
		this->m_IsValid = false;
		return;
	}

	//this->m_Rom.insert(this->m_Rom.begin(),
	//	std::istream_iterator<unsigned char>(file),
	//	std::istream_iterator<unsigned char>());

	file.read(reinterpret_cast<char*> (&m_Rom[0]), (romSize - 2) * sizeof(m_Rom[0]));

	std::string sizeLogTxt = "ROM file of size: " + std::to_string(romSize);
	Log::LogInfo(sizeLogTxt.c_str());

	file.close();

	if (this->m_Rom.empty())
	{
		Log::LogError("Error while copying ROM file!");
		this->m_IsValid = false;
		return;
	}
	
	Log::LogInfo("Loaded ROM succesfully!");
	this->m_IsValid = true;
}

/* Return the byte at the address in cartridge ROM (takes into account memory banking)
*  [address] -> Memory address to access
*/
unsigned char Cartridge::ReadU8(int address)
{
	if (address >= 0x4000 && m_Hardware.mapper == Mapper::MBC1) // In banked area
	{
		unsigned short bankAddress = address - 0x4000;
		return m_Rom[bankAddress + (m_ROMBank * (unsigned short)0x4000)];
	}
	
	return m_Rom[address];
}

/* Return two bytes starting at the address in cartridge ROM (takes into account memory banking)
*  [address] -> Memory address to access
*/
unsigned short Cartridge::ReadU16(int address)
{
	if (address >= 0x4000 && m_Hardware.mapper == Mapper::MBC1) // In banked area
	{
		unsigned short bankAddress = address - 0x4000;

		unsigned char lsb = m_Rom[bankAddress + (m_ROMBank * (unsigned short)0x4000)];
		unsigned char msb = m_Rom[bankAddress + 1 + (m_ROMBank * (unsigned short)0x4000)];

		return ((unsigned short)msb << 8) | lsb;
	}

	unsigned char lsb = m_Rom[address];
	unsigned char msb = m_Rom[address + 1];

	return ((unsigned short)msb << 8) | lsb;
}

/* Try to write in ROM to access mapper registers.
*  [address] -> Memory address to write to
*  [value] -> Value to write at address
*/
void Cartridge::CheckROMWrite(int address, unsigned char value)
{
	if(m_Hardware.mapper == Mapper::MBC1)
	{
		if (address >= 0x2000 && address <= 0x3FFF) // ROM Bank switch
		{
			m_ROMBank = (value == 0) ? 1 : (value & 0x00011111);
		}
		else if (address >= 0x4000 && address <= 0x5FFF && m_Rom[0x0148] >= 0x05) // Second RAM/ROM Bank switch (only if ROM > 1MiB)
		{
			// TO-DO
		}
	}
}

/* Set mapper and internal cartridge addons (ram, battery, timer, rumble & sensor).
*/
void Cartridge::SetHardware(Mapper mapper, bool ram, bool battery, bool timer, bool rumble, bool sensor)
{
	m_Hardware.mapper = mapper;
	m_Hardware.hasRam = ram;
	m_Hardware.hasBattery = battery;
	m_Hardware.hasTimer = timer;
	m_Hardware.hasRumble = rumble;
	m_Hardware.hasSensor = sensor;
}