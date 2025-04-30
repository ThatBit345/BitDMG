#pragma once
#include <memory>

#include "Memory.h"

struct Registers
{
	unsigned char a = 0x01;
	unsigned char b = 0x00;
	unsigned char c = 0x00;
	unsigned char d = 0x00;
	unsigned char e = 0x00;
	unsigned char f = 0x00;
	unsigned char h = 0x00;
	unsigned char l = 0x00;
};

struct FlagRegister
{
	bool zero = 0;
	bool subtract = 0;
	bool halfCarry = 0;
	bool carry = 0;

	// 8 Bits -> ZSHC0000
	unsigned char toU8() const
	{
		unsigned char ret = 0x00;
		ret |= zero << 7;
		ret |= subtract << 6;
		ret |= halfCarry << 5;
		ret |= carry << 4;

		return ret;
	}
};

class CPU
{
private:
	Registers registers;
	FlagRegister flagRegister;

	unsigned short sp;
	unsigned short pc;

	std::shared_ptr<Memory> p_mem;

public: 

	CPU(std::shared_ptr<Memory> memory);

	void Cycle();

	#pragma region DOUBLE REGISTERS
	void SetAF(unsigned short value);
	void SetBC(unsigned short value);
	void SetDE(unsigned short value);
	void SetHL(unsigned short value);

	unsigned short GetAF();
	unsigned short GetBC();
	unsigned short GetDE();
	unsigned short GetHL();
	#pragma endregion



};