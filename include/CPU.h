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
	bool zero = false;
	bool subtract = false;
	bool halfCarry = false;
	bool carry = false;

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

	/* Cycle the CPU to run the next opcode.
	 * @return Number of M-Cycles the opcode took.
	 */
	int Cycle();

	/* Check IE & IF to see if there are any pending interrupts & go to the corresponding handler if requested.
	 * @return M-Cycles taken by the interrupt request.
	 */
	int CheckInterrupts();

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

	/* Set value to 8-bit register.
	 *  @param reg Register ID (B, C, D, E, H, L, memory at HL, A).
	 *  @param value Value to write to register.
	 */
	void SetR8(unsigned char reg, unsigned char value);

	/* Get value at 8-bit register.
	 *  @param reg Register ID (B, C, D, E, H, L, memory at HL, A).
	 *  @return Value at 8-bit register.
	 */
	unsigned char GetR8(unsigned char reg);

	/* Set value to 16-bit register.
	 *  @param reg Register ID (BC, DE, HL, SP).
	 *  @param value Value to write to register.
	 */
	void SetR16(unsigned char reg, unsigned short value);

	/* Set value to 16-bit register.
	 *  @param reg Register ID (BC, DE, HL, SP).
	 *  @return Value at 16-bit register.
	 */
	unsigned short GetR16(unsigned char reg);

	/* Log CPU state to file.
	 */
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

	/* Check if opcode is in valid region.
	 */
	bool IsValidOpcode(unsigned char opcode);

	/* Stop execution and print opcode.
	 */
	int UninplementedOpcode(int opcode);

	int NOP();							 // NOP
	int LD_r16_imm16(unsigned char reg); // LD r16, imm16
	int LD_r16_a(unsigned char reg);	 // LD [r16mem], a
	int LD_a_r16(unsigned char reg);	 // LD a, [r16mem]
	int LD_imm16_SP();					 // LD [imm16], SP
	int INC_r16(unsigned char reg);		 // INC r16
	int DEC_r16(unsigned char reg);		 // DEC r16
	int ADD_HL_r16(unsigned char reg);	 // ADD HL, r16
	int INC_r8(unsigned char reg);		 // INC r8
	int DEC_r8(unsigned char reg);		 // DEC r8
	int LD_r8_imm8(unsigned char reg);	 // LD r8, imm8
	int RLCA();							 // RLCA
	int RRCA();							 // RRCA
	int RLA();							 // RLA
	int RRA();							 // RRA
	int DAA();							 // DAA
	int CPL();							 // CPL
	int SCF();							 // SCF
	int CCF();							 // CCF
	int JR_s8();						 // JR imm8
	int JR_C(unsigned char cond);		 // JR cond, imm8
	int STOP();							 // STOP

	int LD_r8_r8(unsigned char r1, unsigned char r2); // LD r8, r8
	int HALT();										  // HALT

	int ADD_a_r8(unsigned char reg); // ADD A, r8
	int ADC_a_r8(unsigned char reg); // ADC A, r8
	int SUB_a_r8(unsigned char reg); // SUB A, r8
	int SBC_a_r8(unsigned char reg); // SBC A, r8
	int AND_a_r8(unsigned char reg); // AND A, r8
	int XOR_a_r8(unsigned char reg); // XOR A, r8
	int OR_a_r8(unsigned char reg);	 // OR A, r8
	int CP_a_r8(unsigned char reg);	 // CP A, r8

	int ADD_a_imm8();					  // ADD A, imm8
	int ADC_a_imm8();					  // ADC A, imm8
	int SUB_a_imm8();					  // SUB A, imm8
	int SBC_a_imm8();					  // SBC A, imm8
	int AND_a_imm8();					  // AND A, imm8
	int XOR_a_imm8();					  // XOR A, imm8
	int OR_a_imm8();					  // OR A, imm8
	int CP_a_imm8();					  // CP A, imm8
	int RET_C(unsigned char cond);		  // RET cond
	int RET();							  // RET
	int RETI();							  // RETI
	int JP_C_imm16(unsigned char cond);	  // JP cond, imm16
	int JP_imm16();						  // JP imm16
	int JP_HL();						  // JP HL
	int CALL_C_imm16(unsigned char cond); // CALL cond, imm16
	int CALL_imm16();					  // CALL imm16
	int RST_tgt3(unsigned char tgt);	  // RST tgt3
	int POP_r16(unsigned char reg);		  // POP r16stk
	int PUSH_r16(unsigned char reg);	  // PUSH r16stk
	int LDH_c_a();						  // LDH [C], A
	int LDH_imm8_a();					  // LDH [imm8], A
	int LD_imm16_a();					  // LD [imm16], A
	int LDH_a_c();						  // LDH A, [C]
	int LDH_a_imm8();					  // LDH A, [imm8]
	int LD_a_imm16();					  // LD A, [imm16]
	int ADD_SP_imm8();					  // ADD SP, imm8
	int LD_HL_SLimm8();					  // LD HL, SP + imm8
	int LD_SP_HL();						  // LD SP, HL
	int DI();							  // DI
	int EI();							  // EI

	int RLC_r8(unsigned char reg);				   // RLC r8
	int RRC_r8(unsigned char reg);				   // RRC r8
	int RL_r8(unsigned char reg);				   // RL r8
	int RR_r8(unsigned char reg);				   // RR r8
	int SLA_r8(unsigned char reg);				   // SLA r8
	int SRA_r8(unsigned char reg);				   // SRA r8
	int SWAP_r8(unsigned char reg);				   // SWAP r8
	int SRL_r8(unsigned char reg);				   // SRL r8
	int BIT(unsigned char bit, unsigned char reg); // BIT b3, r8
	int RES(unsigned char bit, unsigned char reg); // RES b3, r8
	int SET(unsigned char bit, unsigned char reg); // SET b3, r8
#pragma endregion
};