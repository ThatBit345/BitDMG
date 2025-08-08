#pragma once

#include <vector>
#include <string>
#include <filesystem>

enum class Mapper
{
	None = 0,
	MBC1,
	MBC2,
	MBC3,
	MBC5,
	MBC6,
	MBC7,
	MMM01,
	HuC1,
	HuC3
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

	/* Checks if the cartridge has been loaded correctly.
	 * @returns True if the cartridge is loaded correctly.
	 */
	inline bool IsValid() { return m_IsValid; }

	/* Fetch cartridge mapper.
	 * @returns Mapper in use.
	 */
	inline Mapper GetMapper() { return m_Hardware.mapper; }

	/* Fetch cartridge name.
	 * @returns Name in the cartridge's header.
	 */
	inline std::string GetCartName() { return m_CartName; }

	/* Get the byte at the address in cartridge ROM (takes into account memory banking).
	 *  @param address Memory address to access.
	 *  @return Byte at memory address.
	 */
	unsigned char ReadU8(int address);

	/* Get the two bytes starting at the address in cartridge ROM (takes into account memory banking).
	 *  @param address Memory address of first byte.
	 *  @return Two bytes at memory address.
	 */
	unsigned short ReadU16(int address);
	/* Get the byte at the address in cartridge RAM.
	 *  @param address Memory address to access.
	 *  @return Byte at memory address.
	 */
	unsigned char ReadU8RAM(int address);

	/* Get the two bytes starting at the address in cartridge RAM.
	 *  @param address Memory address of first byte.
	 *  @return Two bytes at memory address.
	 */
	unsigned char ReadU16RAM(int address);

	/* Write to the byte at the address in cartridge RAM.
	 *  @param address Memory address to access.
	 *  @param value Value to write at memory address.
	 */
	void WriteU8RAM(int address, unsigned char value);


	/* Write the two bytes to the address and the next address in cartridge RAM.
	 *  @param address Memory address to access.
	 *  @param value Value to write at memory address.
	 */
	void WriteU16RAM(int address, unsigned short value);

	/* Write the two bytes to the address and the next address in cartridge RAM.
	 *  @param address Memory address to access.
	 *  @param lsb Least significant bit of value to write at memory address.
	 *  @param msb Most significant bit of value to write at memory address + 1.
	 */
	void WriteU16RAM(int address, unsigned char lsb, unsigned char msb);

	/* Try to write in ROM to access mapper registers.
	 *  @param address Memory address to write to.
	 *  @param value Value to write at address.
	 */
	void CheckROMWrite(int address, unsigned char value);

private:
	std::vector<unsigned char> m_Rom;
	std::vector<unsigned char> m_Ram;
	std::string m_CartName;
	std::filesystem::path m_SaveFile;
	CartridgeHardware m_Hardware;

	unsigned char m_RomBank;
	bool m_IsValid;

	bool m_RamEnabled;

	/* Set mapper and internal cartridge addons (ram, battery, timer, rumble & sensor).
	 */
	void SetHardware(Mapper mapper, bool ram, bool battery, bool timer, bool rumble, bool sensor);

	/* Save contents of the cartridge's RAM to a file.
	 */
	void SaveGameToFile();
};