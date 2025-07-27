#pragma once

#include <vector>
#include <string>
#include <filesystem>

enum class Mapper
{
	None = 0, MBC1, MBC2, MBC3, MBC5, MBC6, MBC7, MMM01, HuC1, HuC3
};

struct CartridgeHardware
{
	Mapper mapper = Mapper::None;
	bool hasRam = false;
	bool hasTimer = false;
	bool hasBattery = false;
	bool hasRumble = false;
	bool hasSensor = false;
};

class Cartridge
{
public:
	Cartridge(std::filesystem::path romPath);

	inline bool IsValid() { return m_IsValid; }
	inline Mapper GetMapper() { return m_Hardware.mapper; }
	inline std::string GetCartName() { return m_CartName; }

	unsigned char ReadU8(int address);
	unsigned short ReadU16(int address);

	unsigned char ReadU8RAM(int address);
	unsigned char ReadU16RAM(int address);
	void WriteU8RAM(int address, unsigned char value);
	void WriteU16RAM(int address, unsigned short value);
	void WriteU16RAM(int address, unsigned char lsb, unsigned char msb);

	void CheckROMWrite(int address, unsigned char value);

private:
	std::vector<unsigned char> m_Rom;
	std::vector<unsigned char> m_Ram;
	std::string m_CartName;
	std::filesystem::path m_SaveFile;
	CartridgeHardware m_Hardware;

	unsigned char m_ROMBank;
	bool m_IsValid;

	bool m_RAMEnabled;

	void SetHardware(Mapper mapper, bool ram, bool battery, bool timer, bool rumble, bool sensor);

	void SaveGameToFile();
};