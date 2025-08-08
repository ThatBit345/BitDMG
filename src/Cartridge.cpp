#include "Cartridge.h"

#include <string>
#include <iostream>
#include <fstream>

#include <array>

#include "Log.h"
#include <cmath>

Cartridge::Cartridge(std::filesystem::path romPath) : m_RomBank(1), m_RamEnabled(false)
{
	std::string logTxt = "Loading ROM file: " + romPath.string();
	Log::LogInfo(logTxt.c_str());

	std::ifstream file(romPath, std::ios::out | std::ios::binary);
	if (!file)
	{
		Log::LogError("Failed to open ROM file!");
		this->m_IsValid = false;
	}

	m_SaveFile = romPath.filename().replace_extension(".sav");

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

	// Set RAM size
	switch(header[0x0149])
	{
		case 0:
		case 1:
			m_Ram.resize(0);
			break;

		case 2: // 8KiB
			m_Ram.resize(8192);
			break;
	
		case 3: // 32KiB
			m_Ram.resize(32768);
			break;
	
		case 4: // 128KiB
			m_Ram.resize(131072);
			break;
	
		case 5: // 64KiB
			m_Ram.resize(65536);
			break;
	}

	std::string ramLogTxt = "RAM size: " + std::to_string(m_Ram.size());
	Log::LogInfo(ramLogTxt.c_str());

	// Load save file
	if (m_Ram.size() > 0)
	{
		if(std::filesystem::exists(m_SaveFile))
		{
			std::ifstream save(m_SaveFile, std::ios::out | std::ios::binary);
			if (!save)
			{
				Log::LogError("Could not open save file!");
			}

			save.seekg(0, std::ios::beg);
			save.read(reinterpret_cast<char*> (&m_Ram[0]), m_Ram.size() * sizeof(m_Ram[0]));
			save.close();
		}
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

unsigned char Cartridge::ReadU8(int address)
{
	if (m_Hardware.mapper == Mapper::MBC1)
	{
		if(address < 0x4000) 
			return m_Rom[address];

		int bankedAddress = (address - 0x4000) + (m_RomBank * 0x4000);
		return m_Rom[bankedAddress];
	}
	
	return m_Rom[address];
}

unsigned short Cartridge::ReadU16(int address)
{
	if (m_Hardware.mapper == Mapper::MBC1)
	{
		if(address < 0x4000) 
		{
			unsigned char lsb = m_Rom[address];
			unsigned char msb = m_Rom[address + 1];

			return ((unsigned short)msb << 8) | lsb;
		}

		int bankedAddress = (address - 0x4000) + (m_RomBank * 0x4000);

		unsigned char lsb = m_Rom[bankedAddress];
		unsigned char msb = m_Rom[bankedAddress + 1];

		return ((unsigned short)msb << 8) | lsb;
	}

	unsigned char lsb = m_Rom[address];
	unsigned char msb = m_Rom[address + 1];

	return ((unsigned short)msb << 8) | lsb;
}

unsigned char Cartridge::ReadU8RAM(int address)
{
	if(!m_RamEnabled) return 0xFF;

	if(m_Hardware.mapper == Mapper::MBC1)
	{
		return m_Ram[address - 0xA000];
	}

	return 0xFF;
}

unsigned char Cartridge::ReadU16RAM(int address)
{
	unsigned char lsb = m_Ram[address - 0xA000];
	unsigned char msb = m_Ram[address - 0xA000 + 1];

	return ((unsigned short)msb << 8) | lsb;
}

void Cartridge::WriteU8RAM(int address, unsigned char value)
{
	if(!m_RamEnabled) return;

	if(m_Hardware.mapper == Mapper::MBC1)
	{
		m_Ram[address - 0xA000] = value;
	}

	SaveGameToFile();
}

void Cartridge::WriteU16RAM(int address, unsigned short value)
{
	if(!m_RamEnabled) return;

	if(m_Hardware.mapper == Mapper::MBC1)
	{
		m_Ram[address - 0xA000] = value & 0xFF;
		m_Ram[address - 0xA000 + 1] = value >> 8;
	}

	SaveGameToFile();
}

void Cartridge::WriteU16RAM(int address, unsigned char lsb, unsigned char msb)
{
	if(!m_RamEnabled) return;

	if(m_Hardware.mapper == Mapper::MBC1)
	{
		m_Ram[address - 0xA000] = lsb;
		m_Ram[address - 0xA000 + 1] = msb;
	}

	SaveGameToFile();
}

void Cartridge::CheckROMWrite(int address, unsigned char value)
{
	if(m_Hardware.mapper == Mapper::MBC1)
	{
		if(address <= 0x1FFF) // RAM Enable
		{
			m_RamEnabled = (value & 0b1111) == 0xA;
		}
		else if (address >= 0x2000 && address <= 0x3FFF) // ROM Bank switch
		{
			m_RomBank = (value == 0) ? 1 : (value & 0b00011111);
		}
		else if (address >= 0x4000 && address <= 0x5FFF && m_Rom[0x0148] >= 0x05) // Second RAM/ROM Bank switch (only if ROM > 1MiB)
		{
			// TO-DO
		}
	}
}

void Cartridge::SetHardware(Mapper mapper, bool ram, bool battery, bool timer, bool rumble, bool sensor)
{
	m_Hardware.mapper = mapper;
	m_Hardware.hasRam = ram;
	m_Hardware.hasBattery = battery;
	m_Hardware.hasTimer = timer;
	m_Hardware.hasRumble = rumble;
	m_Hardware.hasSensor = sensor;
}

void Cartridge::SaveGameToFile()
{
	std::ofstream save(m_SaveFile, std::ios::out | std::ios::binary);
	if (!save)
	{
		Log::LogError("Could not save game!");
	}

	for (size_t i = 0; i < m_Ram.size(); i++)
	{
		save << m_Ram[i];
	}

	save.close();
}
