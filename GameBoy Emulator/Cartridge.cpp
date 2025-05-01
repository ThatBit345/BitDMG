#include "Cartridge.h"

#include <string>
#include <iostream>
#include <fstream>

#include <array>

#include "Log.h"

Cartridge::Cartridge(const char* romPath)
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
		SetHardware(Mapper::MBC2, false, false, false, false, false);
		break;

	case 0x06: // MBC2 + BATTERY
		SetHardware(Mapper::MBC2, false, true, false, false, false);
		break;

	case 0x08: // ROM + RAM
		SetHardware(Mapper::None, true, false, false, false, false);
		break;

	case 0x09: // ROM + RAM + BATTERY
		SetHardware(Mapper::None, true, true, false, false, false);
		break;

	case 0x0B: // MMM01
		SetHardware(Mapper::MMM01, false, false, false, false, false);
		break;

	case 0x0C: // MMM01 + RAM
		SetHardware(Mapper::MMM01, true, false, false, false, false);
		break;

	case 0x0D: // MMM01 + RAM + BATTERY
		SetHardware(Mapper::MMM01, true, true, false, false, false);
		break;

	case 0x0F: // MBC3 + BATTERY + TIMER
		SetHardware(Mapper::MBC3, false, true, true, false, false);
		break;

	case 0x10: // MBC3 + RAM + BATTERY + TIMER
		SetHardware(Mapper::MBC3, true, true, true, false, false);
		break;

	case 0x11: // MBC3
		SetHardware(Mapper::MBC3, false, false, false, false, false);
		break;

	case 0x12: // MBC3 + RAM
		SetHardware(Mapper::MBC3, true, false, false, false, false);
		break;

	case 0x13: // MBC3 + RAM + BATTERY
		SetHardware(Mapper::MBC3, true, true, false, false, false);
		break;

	case 0x19: // MBC5
		SetHardware(Mapper::MBC5, false, false, false, false, false);
		break;

	case 0x1A: // MBC5 + RAM
		SetHardware(Mapper::MBC5, true, false, false, false, false);
		break;

	case 0x1B: // MBC5 + RAM + BATTERY
		SetHardware(Mapper::MBC5, true, true, false, false, false);
		break;

	case 0x1C: // MBC5 + RUMBLE
		SetHardware(Mapper::MBC5, false, false, false, true, false);
		break;

	case 0x1D: // MBC5 + RAM + RUMBLE
		SetHardware(Mapper::MBC5, true, false, false, true, false);
		break;

	case 0x1E: // MBC5 + RAM + BATTERY + RUMBLE
		SetHardware(Mapper::MBC5, true, true, false, true, false);
		break;

	case 0x20: // MBC6
		SetHardware(Mapper::MBC6, false, false, false, false, false);
		break;

	case 0x22: // MBC7 + RAM + BATTERY + RUMBLE + SENSOR
		SetHardware(Mapper::MBC7, true, true, false, true, true);
		break;

	case 0xFE: // HuC3
		SetHardware(Mapper::HuC3, false, false, false, false, false);
		break;

	case 0xFF: // HuC1 + RAM + BATTERY
		SetHardware(Mapper::HuC1, true, true, false, false, false);
		break;
	}

	// Calculate the size of the rom specified in the header
	size_t romSize = 32768 * std::pow(2, header[0x0148]);
	this->m_Rom.reserve(romSize);

	// Copy ROM into cartridge memory
	file.seekg(0, std::ios::beg);

	if(!file.good())
	{
		Log::LogError("Error while reading ROM file!");
		this->m_IsValid = false;
		return;
	}

	this->m_Rom.insert(this->m_Rom.begin(),
		std::istream_iterator<unsigned char>(file),
		std::istream_iterator<unsigned char>());

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

unsigned char* Cartridge::GetRom()
{
	return this->m_Rom.data();
}

void Cartridge::SetHardware(Mapper mapper, bool ram, bool battery, bool timer, bool rumble, bool sensor)
{
	this->m_Hardware.mapper = mapper;
	this->m_Hardware.hasRam = ram;
	this->m_Hardware.hasBattery = battery;
	this->m_Hardware.hasTimer = timer;
	this->m_Hardware.hasRumble = rumble;
	this->m_Hardware.hasSensor = sensor;
}