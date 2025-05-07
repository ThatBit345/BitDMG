#pragma once
#include <array>
#include <memory>

#include "Cartridge.h"

class Memory
{
public:
	Memory(std::shared_ptr<Cartridge> cart);

	unsigned char ReadU8(int address);
	void WriteU8(int address, unsigned char value);

	unsigned short ReadU16(int address);
	unsigned short ReadU16Stack(int address);
	void WriteU16(int address, unsigned short value);
	void WriteU16(int address, unsigned char lsb, unsigned char msb);

private:
	std::array<unsigned char, 0x10000> m_Memory;

	std::shared_ptr<Cartridge> m_Cartridge;
};

