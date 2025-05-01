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
	unsigned char toU8()
	{
		unsigned char ret = 0x00;
		ret |= this->zero << 7;
		ret |= this->subtract << 6;
		ret |= this->halfCarry << 5;
		ret |= this->carry << 4;

		return ret;
	}

	void fromU8(unsigned char byte)
	{
		this->zero = ((byte & 0b10000000) >> 7 == 1) ? true : false;
		this->subtract = ((byte & 0b01000000) >> 6 == 1) ? true : false;
		this->halfCarry = ((byte & 0b00100000) >> 5 == 1) ? true : false;
		this->carry = ((byte & 0b00010000) >> 4 == 1) ? true : false;
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

	bool Cycle();

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

	#pragma region OPCODES

	void NOP();						// NOP
	void LD_SP();					// LD address, SP
	void STOP();					// STOP
	void JR_s8();					// JR s8
	void JR_C(unsigned char cond);	// JR cond
	void LD_RP(unsigned char reg);	// LD to register pair (with SP)
	void ADD_HL(unsigned char reg);	// ADD HL

	#pragma endregion

};