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

	void reset()
	{
		this->zero = false;
		this->subtract = false;
		this->halfCarry = false;
		this->carry = false;
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

	void UninplementedOpcode(int opcode);

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

	void NOP();											// NOP
	void LD_r16_imm16(unsigned char reg);				// LD r16, imm16
	void LD_r16_a(unsigned char reg);					// LD [r16mem], a
	void LD_a_r16(unsigned char reg);					// LD a, [r16mem]
	void LD_imm16_SP();									// LD [imm16], SP
	void INC_r16(unsigned char reg);					// INC r16
	void DEC_r16(unsigned char reg);					// DEC r16
	void ADD_HL_r16(unsigned char reg);					// ADD HL, r16
	void INC_r8(unsigned char reg);						// INC r8
	void DEC_r8(unsigned char reg);						// DEC r8
	void LD_r8_imm8(unsigned char reg);					// LD r8, imm8
	void RLCA();										// RLCA
	void RRCA();										// RRCA
	void RLA();											// RLA
	void RRA();											// RRA
	void DAA();											// DAA
	void CPL();											// CPL
	void SCF();											// SCF
	void CCF();											// CCF
	void JR_s8();										// JR imm8
	void JR_C(unsigned char cond);						// JR cond, imm8
	void STOP();										// STOP

	void LD_r8_r8(unsigned char r1, unsigned char r2);	// LD r8, r8
	void HALT();										// HALT

	#pragma endregion

};