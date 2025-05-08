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
public:
	CPU(std::shared_ptr<Memory> memory);

	// Returns number of cycles elapsed (in machine cycles)
	int Cycle();
	void CheckInterrupts();

private:
	Registers m_Registers;
	FlagRegister m_FlagRegister;

	unsigned short m_SP;
	unsigned short m_PC;
	bool m_IME;
	bool m_EnableIME;
	bool m_Halted;
	bool m_HaltBug;

	std::shared_ptr<Memory> m_Mem;

	void SetR8(unsigned char reg, unsigned char value);
	unsigned char GetR8(unsigned char reg);

	void SetR16(unsigned char reg, unsigned short value);
	unsigned short GetR16(unsigned char reg);

	void Log();

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
	bool IsValidOpcode(unsigned char opcode);
	int UninplementedOpcode(int opcode);

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

	void ADD_a_r8(unsigned char reg);					// ADD A, r8
	void ADC_a_r8(unsigned char reg);					// ADC A, r8
	void SUB_a_r8(unsigned char reg);					// SUB A, r8
	void SBC_a_r8(unsigned char reg);					// SBC A, r8
	void AND_a_r8(unsigned char reg);					// AND A, r8
	void XOR_a_r8(unsigned char reg);					// XOR A, r8
	void OR_a_r8(unsigned char reg);					// OR A, r8
	void CP_a_r8(unsigned char reg);					// CP A, r8

	void ADD_a_imm8();									// ADD A, imm8
	void ADC_a_imm8();									// ADC A, imm8
	void SUB_a_imm8();									// SUB A, imm8
	void SBC_a_imm8();									// SBC A, imm8
	void AND_a_imm8();									// AND A, imm8
	void XOR_a_imm8();									// XOR A, imm8
	void OR_a_imm8();									// OR A, imm8
	void CP_a_imm8();									// CP A, imm8
	void RET_C(unsigned char cond);						// RET cond
	void RET();											// RET
	void RETI();										// RETI
	void JP_C_imm16(unsigned char cond);				// JP cond, imm16
	void JP_imm16();									// JP imm16	
	void JP_HL();										// JP HL
	void CALL_C_imm16(unsigned char cond);				// CALL cond, imm16
	void CALL_imm16();									// CALL imm16
	void RST_tgt3(unsigned char tgt);					// RST tgt3
	void POP_r16(unsigned char reg);					// POP r16stk
	void PUSH_r16(unsigned char reg);					// PUSH r16stk
	void LDH_c_a();										// LDH [C], A
	void LDH_imm8_a();									// LDH [imm8], A
	void LD_imm16_a();									// LD [imm16], A
	void LDH_a_c();										// LDH A, [C]
	void LDH_a_imm8();									// LDH A, [imm8]
	void LD_a_imm16();									// LD A, [imm16]
	void ADD_SP_imm8();									// ADD SP, imm8
	void LD_HL_SLimm8();								// LD HL, SP + imm8
	void LD_SP_HL();									// LD SP, HL
	void DI();											// DI
	void EI();											// EI

	void RLC_r8(unsigned char reg);						// RLC r8
	void RRC_r8(unsigned char reg);						// RRC r8
	void RL_r8(unsigned char reg);						// RL r8
	void RR_r8(unsigned char reg);						// RR r8
	void SLA_r8(unsigned char reg);						// SLA r8
	void SRA_r8(unsigned char reg);						// SRA r8
	void SWAP_r8(unsigned char reg);					// SWAP r8
	void SRL_r8(unsigned char reg);						// SRL r8
	void BIT(unsigned char bit, unsigned char reg);		// BIT b3, r8
	void RES(unsigned char bit, unsigned char reg);		// RES b3, r8
	void SET(unsigned char bit, unsigned char reg);		// SET b3, r8
	#pragma endregion

};