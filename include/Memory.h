#pragma once
#include <array>
#include <memory>

#include "Cartridge.h"

class Memory
{
public:
	Memory(std::shared_ptr<Cartridge> cart);

	unsigned char ReadU8(unsigned short address);
	unsigned char ReadU8Unfiltered(unsigned short address);
	void WriteU8(unsigned short address, unsigned char value);
	void WriteU8Unfiltered(unsigned short address, unsigned char value);

	unsigned short ReadU16(unsigned short address);
	void WriteU16(unsigned short address, unsigned short value);
	void WriteU16(unsigned short address, unsigned char lsb, unsigned char msb);
	void WriteU16Unfiltered(unsigned short address, unsigned char value);
	void WriteU16Stack(unsigned short address, unsigned short value);

	void LockVRAM();
	void UnlockVRAM();
	void LockOAM();
	void UnlockOAM();

	void UpdateInputState(bool buffer[8]);

private:
	std::array<unsigned char, 0x10000> m_Memory;

	std::shared_ptr<Cartridge> m_Cartridge;

	bool m_InputBuffer[8];
	void UpdateInputRegister();
	void HandleInputInterrupt(bool isButtons);

	bool m_VRAMLocked;
	bool m_OAMLocked;
};

enum InputButtons
{
	RIGHT = 0,
	LEFT,
	UP,
	DOWN,
	A,
	B,
	SELECT,
	START
};