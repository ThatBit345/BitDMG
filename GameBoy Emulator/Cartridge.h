#pragma once
#include <vector>
#include <string>

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
	Cartridge(const char* romPath);

	inline bool IsValid() { return m_IsValid; }
	unsigned char ReadU8(int address);
	unsigned short ReadU16(int address);
	void CheckROMWrite(int address, unsigned char value);
	inline Mapper GetMapper() { return m_Hardware.mapper; }

private:
	std::vector<unsigned char> m_Rom;
	std::string m_CartName;
	CartridgeHardware m_Hardware;

	unsigned char m_ROMBank;
	bool m_IsValid;

	void SetHardware(Mapper mapper, bool ram, bool battery, bool timer, bool rumble, bool sensor);

};