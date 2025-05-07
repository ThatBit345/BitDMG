#include "CPU.h"
#include "Log.h"

#include <iostream>
#include <sstream>
#include <iomanip>

CPU::CPU(std::shared_ptr<Memory> memory) : m_SP(0xFFFE), m_PC(0x0100)
{
	// Mimic state after boot ROM
	m_Registers.a = 0x01;
	m_Registers.b = 0x00;
	m_Registers.c = 0x13;
	m_Registers.d = 0x00;
	m_Registers.e = 0xD8;
	m_Registers.h = 0x01;
	m_Registers.l = 0x4D;

	m_FlagRegister.zero = true;
	m_FlagRegister.halfCarry = true;
	m_FlagRegister.carry = true;

	m_IME = false;
	m_EnableIME = false;

	m_Mem = memory;
}

bool CPU::Cycle()
{
	Log();

	unsigned char opcode = m_Mem->ReadU8(m_PC++);

	if(!IsValidOpcode(opcode))
	{
		m_PC--;
		return false;
	}

	// Opcode decoding, method described by Scott Mansell in the website below
	// https://archive.gbdev.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
	// and using the reference table described in the pandocs
	// https://gbdev.io/pandocs/CPU_Instruction_Set.html

	unsigned char block = opcode >> 6;
	unsigned char y = (opcode & 0b00111000) >> 3;
	unsigned char z = opcode & 0b00000111;

	unsigned char p = (opcode & 0b00110000) >> 4;
	bool q = (opcode & 0b00001000) >> 3;

	if(opcode == 0xCB)
	{
		opcode = m_Mem->ReadU8(m_PC++);

		// Recalculate sections
		block = opcode >> 6;
		y = (opcode & 0b00111000) >> 3;
		z = opcode & 0b00000111;

		if (block == 0)
		{
			if (y == 0) RLC_r8(z);
			else if (y == 1) RRC_r8(z);
			else if (y == 2) RL_r8(z);
			else if (y == 3) RR_r8(z);
			else if (y == 4) SLA_r8(z);
			else if (y == 5) SRA_r8(z);
			else if (y == 6) SWAP_r8(z);
			else if (y == 7) SRL_r8(z);
			else return UninplementedOpcode(opcode);
		}
		else if (block == 1) BIT(y, z);
		else if (block == 2) RES(y, z);
		else if (block == 3) SET(y, z);
		else return UninplementedOpcode(opcode);
	}
	else if(block == 0)
	{
		if(z == 0)
		{
			if (y == 0) NOP();
			else if (y == 1) LD_imm16_SP();
			else if (y == 2) STOP();
			else if (y == 3) JR_s8();
			else JR_C(y - 4);
		}
		else if(z == 1)
		{
			if (q == 0) LD_r16_imm16(p);
			else if (q == 1) ADD_HL_r16(p);
			else return UninplementedOpcode(opcode);
		}
		else if(z == 2)
		{
			if (q == 0) LD_r16_a(p);
			else if (q == 1) LD_a_r16(p);
			else return UninplementedOpcode(opcode);
		}
		else if(z == 3)
		{
			if (q == 0) INC_r16(p);
			else if (q == 1) DEC_r16(p);
			else return UninplementedOpcode(opcode);
		}
		else if(z == 4) INC_r8(y);
		else if(z == 5) DEC_r8(y);
		else if(z == 6) LD_r8_imm8(y);
		else if(z == 7)
		{
			if (y == 0) RLCA();
			else if (y == 1) RRCA();
			else if (y == 2) RLA();
			else if (y == 3) RRA();
			else if (y == 4) DAA();
			else if (y == 5) CPL();
			else if (y == 6) SCF();
			else if (y == 7) CCF();
			else return UninplementedOpcode(opcode);
		}
		else return UninplementedOpcode(opcode);
	}
	else if (block == 1)
	{
		if (opcode == 0x76) HALT();
		else LD_r8_r8(y, z);
	}
	else if (block == 2)
	{
		if (y == 0) ADD_a_r8(z);
		else if (y == 1) ADC_a_r8(z);
		else if (y == 2) SUB_a_r8(z);
		else if (y == 3) SBC_a_r8(z);
		else if (y == 4) AND_a_r8(z);
		else if (y == 5) XOR_a_r8(z);
		else if (y == 6) OR_a_r8(z);
		else if (y == 7) CP_a_r8(z);
		else return UninplementedOpcode(opcode);
	}
	else if (block == 3)
	{
		if (z == 0)
		{
			if (y <= 3) RET_C(y);
			else if (y == 4) LDH_imm8_a();
			else if (y == 5) ADD_SP_imm8();
			else if (y == 6) LDH_a_imm8();
			else if (y == 7) LD_HL_SLimm8();
			else return UninplementedOpcode(opcode);
		}
		else if (z == 1)
		{
			if (q == 0) POP_r16(p);
			else if (y == 1) RET();
			else if (y == 3) RETI();
			else if (y == 5) JP_HL();
			else if (y == 7) LD_SP_HL();
			else return UninplementedOpcode(opcode);
		}
		else if (z == 2)
		{
			if (y <= 3) JP_C_imm16(y);
			else if (y == 4) LDH_c_a();
			else if (y == 5) LD_imm16_a();
			else if (y == 6) LDH_a_c();
			else if (y == 7) LD_a_imm16();
			else return UninplementedOpcode(opcode);
		}
		else if (z == 3)
		{
			if (y == 0) JP_imm16();
			else if (y == 6) DI();
			else if (y == 7) EI();
			else return UninplementedOpcode(opcode);
		}
		else if (z == 4) CALL_C_imm16(y);
		else if (z == 5)
		{
			if (q == 0) PUSH_r16(p);
			else if(y == 1) CALL_imm16();
			else return UninplementedOpcode(opcode);
		}
		else if (z == 6)
		{
			if (y == 0) ADD_a_imm8();
			else if (y == 1) ADC_a_imm8();
			else if (y == 2) SUB_a_imm8();
			else if (y == 3) SBC_a_imm8();
			else if (y == 4) AND_a_imm8();
			else if (y == 5) XOR_a_imm8();
			else if (y == 6) OR_a_imm8();
			else if (y == 7) CP_a_imm8();
			else return UninplementedOpcode(opcode);
		}
		else if (z == 7) RST_tgt3(y);
		else return UninplementedOpcode(opcode);
	}
	else return UninplementedOpcode(opcode);
	
	// Enable interrupts after the instruction (used by the EI instruction)
	if(m_EnableIME)
	{
		m_IME = true;
		m_EnableIME = false;
	}

	return false;
}

void CPU::SetR8(unsigned char reg, unsigned char value)
{
	switch (reg)
	{
	case 0:
		m_Registers.b = value;
		break;

	case 1:
		m_Registers.c = value;
		break;

	case 2:
		m_Registers.d = value;
		break;

	case 3:
		m_Registers.e = value;
		break;

	case 4:
		m_Registers.h = value;
		break;

	case 5:
		m_Registers.l = value;
		break;

	case 6:
		m_Mem->WriteU8(GetHL(), value);
		break;

	case 7:
		m_Registers.a = value;
		break;
	}
}

unsigned char CPU::GetR8(unsigned char reg)
{
	switch (reg)
	{
	case 0:
		return m_Registers.b;

	case 1:
		return m_Registers.c;

	case 2:
		return m_Registers.d;

	case 3:
		return m_Registers.e;

	case 4:
		return m_Registers.h;

	case 5:
		return m_Registers.l;

	case 6:
		return m_Mem->ReadU8(GetHL());

	case 7:
		return m_Registers.a;
	}

	return -1;
}

void CPU::SetR16(unsigned char reg, unsigned short value)
{
	switch (reg)
	{
	case 0:
		SetBC(value);
		break;

	case 1:
		SetDE(value);
		break;

	case 2:
		SetHL(value);
		break;

	case 3:
		m_SP = value;
		break;
	}
}

unsigned short CPU::GetR16(unsigned char reg)
{
	switch (reg)
	{
	case 0:
		return GetBC();

	case 1:
		return GetDE();

	case 2:
		return GetHL();

	case 3:
		return m_SP;
	}

	return -1;
}

void CPU::Log()
{
	std::clog << std::hex << std::uppercase <<
		"A:" << std::setw(2) << std::setfill('0') << (int)m_Registers.a <<
		" F:" << std::setw(2) << std::setfill('0') << (int)m_FlagRegister.toU8() <<
		" B:" << std::setw(2) << std::setfill('0') << (int)m_Registers.b <<
		" C:" << std::setw(2) << std::setfill('0') << (int)m_Registers.c <<
		" D:" << std::setw(2) << std::setfill('0') << (int)m_Registers.d <<
		" E:" << std::setw(2) << std::setfill('0') << (int)m_Registers.e <<
		" H:" << std::setw(2) << std::setfill('0') << (int)m_Registers.h <<
		" L:" << std::setw(2) << std::setfill('0') << (int)m_Registers.l <<
		" SP:" << std::setw(4) << std::setfill('0') << (int)m_SP <<
		" PC:" << std::setw(4) << std::setfill('0') << (int)m_PC <<
		" PCMEM:" << std::setw(2) << std::setfill('0') << (int)m_Mem->ReadU8(m_PC) <<
		"," << std::setw(2) << std::setfill('0') << (int)m_Mem->ReadU8(m_PC + 1) <<
		"," << std::setw(2) << std::setfill('0') << (int)m_Mem->ReadU8(m_PC + 2) <<
		"," << std::setw(2) << std::setfill('0') << (int)m_Mem->ReadU8(m_PC + 3) <<
		std::endl;
}

#pragma region DOUBLE REGISTERS
void CPU::SetAF(unsigned short value)
{
	this->m_Registers.a = value >> 8;
	this->m_FlagRegister.fromU8(value);
}

void CPU::SetBC(unsigned short value)
{
	this->m_Registers.b = value >> 8;
	this->m_Registers.c = value;
}

void CPU::SetDE(unsigned short value)
{
	this->m_Registers.d = value >> 8;
	this->m_Registers.e = value;
}

void CPU::SetHL(unsigned short value)
{
	this->m_Registers.h = value >> 8;
	this->m_Registers.l = value;
}

unsigned short CPU::GetAF()
{
	return ((unsigned short)this->m_Registers.a << 8) | this->m_FlagRegister.toU8();
}

unsigned short CPU::GetBC()
{
	return ((unsigned short)this->m_Registers.b << 8) | this->m_Registers.c;
}

unsigned short CPU::GetDE()
{
	return ((unsigned short)this->m_Registers.d << 8) | this->m_Registers.e;
}

unsigned short CPU::GetHL()
{
	return ((unsigned short)this->m_Registers.h << 8) | this->m_Registers.l;
}
#pragma endregion

bool CPU::IsValidOpcode(unsigned char opcode)
{
	return opcode != 0xD3 &&
		opcode != 0xDB &&
		opcode != 0xDD &&
		opcode != 0xE3 &&
		opcode != 0xE4 &&
		opcode != 0xEB &&
		opcode != 0xEC &&
		opcode != 0xED &&
		opcode != 0xF4 &&
		opcode != 0xFC &&
		opcode != 0xFD;
}

bool CPU::UninplementedOpcode(int opcode)
{
	// Print error to console
	std::string errorTxt = "Opcode not found: ";
	std::stringstream str;
	str << std::hex << std::uppercase << (int)opcode;
	errorTxt = errorTxt + str.str();
	Log::LogError(errorTxt.c_str());

	return true;
}

// No operation
void CPU::NOP()
{
	return;
}

// Copy the immediate value into register r16
void CPU::LD_r16_imm16(unsigned char reg)
{
	unsigned short value = m_Mem->ReadU16(m_PC++);
	m_PC++; // Adjust for the two memory reads for imm16
	SetR16(reg, value);
}

// Copy the value in register A into the byte pointed to by r16 (r16mem)
void CPU::LD_r16_a(unsigned char reg)
{
	unsigned short address = 0x0000;

	switch (reg)
	{
	case 0:
		address = GetBC();
		break;

	case 1:
		address = GetDE();
		break;

	case 2:
		address = GetHL();
		SetHL(GetHL() + 1);
		break;

	case 3:
		address = GetHL();
		SetHL(GetHL() - 1);
		break;
	}

	if (address == 0xFF01) Log::LogInfo((char*)&m_Registers.a);

	m_Mem->WriteU8(address, m_Registers.a);
}

// Copy the byte pointed to by register r16 into A (r16mem)
void CPU::LD_a_r16(unsigned char reg)
{
	unsigned short address = 0x0000;

	switch (reg)
	{
	case 0:
		address = GetBC();
		break;

	case 1:
		address = GetDE();
		break;

	case 2:
		address = GetHL();
		SetHL(GetHL() + 1);
		break;

	case 3:
		address = GetHL();
		SetHL(GetHL() - 1);
		break;
	}

	m_Registers.a = m_Mem->ReadU8(address);
}

// Copy the immediate value into SP
void CPU::LD_imm16_SP()
{
	unsigned char lsb = m_Mem->ReadU8(m_PC++);
	unsigned char msb = m_Mem->ReadU8(m_PC++);

	unsigned short address = ((unsigned short)lsb << 8) | msb;

	if (address == 0xFF01) Log::LogInfo((char*)&m_Registers.a);

	m_Mem->WriteU16(address, m_SP);
}

// Increment value at register r16 by 1
void CPU::INC_r16(unsigned char reg)
{
	SetR16(reg, GetR16(reg) + 1);
}

// Decrement value at register r16 by 1
void CPU::DEC_r16(unsigned char reg)
{
	SetR16(reg, GetR16(reg) - 1);
}

// Add value in HL to value at register r16
void CPU::ADD_HL_r16(unsigned char reg)
{
	unsigned short val = GetR16(reg);

	unsigned short result = GetHL() + val;
	SetHL(result);

	// Set flags
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = ((((result - val) & 0xFFF) + (val & 0xFFF)) & 0x1000) == 0x1000;
	//m_FlagRegister.halfCarry = ((result & 0b0000100000000000) >> 11 == 1);
	m_FlagRegister.carry = ((((result - val) & 0xFFF) + (val & 0xFFF)) & 0x1000) == 0x10000;
	//m_FlagRegister.carry = ((result & 0b1000000000000000) >> 15 == 1);
}

// Increment value at register r8 by 1
void CPU::INC_r8(unsigned char reg)
{
	unsigned char result = GetR8(reg) + 1;
	SetR8(reg, result);

	m_FlagRegister.zero = (result == 0);
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = ((((result - 1) & 0xF) + (1 & 0xF)) & 0x10) == 0x10;
}

// Decrement value at register r8 by 1
void CPU::DEC_r8(unsigned char reg)
{
	unsigned char result = GetR8(reg) - 1;
	SetR8(reg, result);

	m_FlagRegister.zero = (result == 0);
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = ((((result + 1) & 0xF) - (1 & 0xF)) & 0x10) == 0x10;
}

// Copy the immediate value into register r8
void CPU::LD_r8_imm8(unsigned char reg)
{
	unsigned char value = m_Mem->ReadU8(m_PC++);

	SetR8(reg, value);
}

// Rotate A to the left (circular)
void CPU::RLCA()
{
	// Store the right-most byte and add it later to the shifted number
	unsigned char carryByte = (m_Registers.a & 0b10000000) >> 7;
	m_Registers.a = m_Registers.a << 1;
	m_Registers.a |= carryByte;

	m_FlagRegister.reset();
	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.carry = carryByte;
}

// Rotate A to the right (circular)
void CPU::RRCA()
{
	// Store the left-most byte and add it later to the shifted number
	unsigned char carryByte = (m_Registers.a & 0b00000001);
	m_Registers.a = m_Registers.a >> 1;
	m_Registers.a |= (carryByte << 7);

	m_FlagRegister.reset();
	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.carry = carryByte;
}

// Rotate A to the left THROUGH the carry flag
void CPU::RLA()
{
	unsigned char oldCarry = m_FlagRegister.carry;
	unsigned char carryByte = (m_Registers.a & 0b10000000);
	m_Registers.a = m_Registers.a >> 1;
	m_Registers.a |= oldCarry;

	m_FlagRegister.reset();
	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.carry = carryByte;
}

// Rotate A to the right THROUGH the carry flag
void CPU::RRA()
{
	unsigned char oldCarry = m_FlagRegister.carry;
	unsigned char carryByte = (m_Registers.a & 0b00000001);
	m_Registers.a = m_Registers.a >> 1;
	m_Registers.a |= (oldCarry << 7);

	m_FlagRegister.reset();
	//m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.carry = carryByte;
}

// Decimal adjust A to binary coded decimal
void CPU::DAA()
{
	//unsigned char adjustment = 0x00;
	//int result;
	//
	//if(m_FlagRegister.subtract)
	//{
	//	if (m_FlagRegister.halfCarry) adjustment += 0x6;
	//	if (m_FlagRegister.carry) adjustment += 0x60;
	//	result = m_Registers.a;
	//	result -= adjustment;
	//	m_Registers.a = result & 0xFF;
	//}
	//else
	//{
	//	if (m_FlagRegister.halfCarry || (m_Registers.a & 0xF) > 0x9) adjustment += 0x6;
	//	if (m_FlagRegister.carry || m_Registers.a > 0x99) adjustment += 0x60;
	//	result = m_Registers.a;
	//	result += adjustment;
	//	m_Registers.a = result & 0xFF;
	//}
	//
	//m_FlagRegister.zero = (m_Registers.a == 0);
	//m_FlagRegister.halfCarry = false;
	//m_FlagRegister.carry = (result & 0x100) == 0x100;

	// Implementation from https://blog.ollien.com/posts/gb-daa/
	unsigned char a = m_Registers.a;
	unsigned char offset = 0x00;

	if((m_FlagRegister.subtract == 0 && (a & 0xF) > 0x09) || m_FlagRegister.halfCarry)
	{
		offset |= 0x06;
	}

	if((m_FlagRegister.subtract == 0 && a > 0x99) || m_FlagRegister.carry)
	{
		offset |= 0x60;
		m_FlagRegister.carry = true;
	}

	if (m_FlagRegister.subtract) a -= offset;
	else a += offset;

	m_Registers.a = a;

	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.halfCarry = false;
}

// Complement A (bitwise NOT)
void CPU::CPL()
{
	m_Registers.a = ~m_Registers.a;

	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = true;
}

// Set the carry flag
void CPU::SCF()
{
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = false;
	m_FlagRegister.carry = true;
}

// Complement the carry flag
void CPU::CCF()
{
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = false;
	m_FlagRegister.carry = !m_FlagRegister.carry;
}

// Jump offset
void CPU::JR_s8() 
{
	signed char offset = m_Mem->ReadU8(m_PC++);
	m_PC += offset;
}

// Jump conditional
void CPU::JR_C(unsigned char cond)
{
	signed char offset = m_Mem->ReadU8(m_PC++);

	if(cond == 0 && !m_FlagRegister.zero) // Not zero
	{
		m_PC += offset;
	}
	else if (cond == 1 && m_FlagRegister.zero) // Zero
	{
		m_PC += offset;
	}
	else if (cond == 2 && !m_FlagRegister.carry) // No carry
	{
		m_PC += offset;
	}
	else if (cond == 3 && m_FlagRegister.carry) // Carry
	{
		m_PC += offset;
	}
}

// Enter CPU very low power mode.
void CPU::STOP()
{
	UninplementedOpcode(0x10);
}

// Copy value from first register into second.
void CPU::LD_r8_r8(unsigned char r1, unsigned char r2)
{
	SetR8(r1, GetR8(r2));
}

// Enter CPU low-power mode.
void CPU::HALT()
{
	UninplementedOpcode(0x76);
}

// Add the value at register r8 and A. Stored in A.
void CPU::ADD_a_r8(unsigned char reg)
{
	unsigned char result = m_Registers.a + GetR8(reg);

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = false;
	//m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	//m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
	m_FlagRegister.halfCarry = ((((result - GetR8(reg)) & 0xF) + (GetR8(reg) & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = ((((result - GetR8(reg)) & 0xF) + (GetR8(reg) & 0xF)) & 0x10) == 0x100;
}

// Add the value at register r8, A and the carry flag. Stored in A.
void CPU::ADC_a_r8(unsigned char reg)
{
	unsigned char result = m_Registers.a + GetR8(reg) + m_FlagRegister.carry;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = false;
	//m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	//m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
	m_FlagRegister.halfCarry = ((((result - GetR8(reg) - m_FlagRegister.carry) & 0xF) + (GetR8(reg) - m_FlagRegister.carry & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = ((((result - GetR8(reg)) - m_FlagRegister.carry & 0xF) + (GetR8(reg) - m_FlagRegister.carry & 0xF)) & 0x10) == 0x100;
}

// Subtract the value at register r8 from A. Stored in A.
void CPU::SUB_a_r8(unsigned char reg)
{
	m_FlagRegister.carry = m_Registers.a < GetR8(reg);

	unsigned char result = m_Registers.a - GetR8(reg);

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	//m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	//m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
	m_FlagRegister.halfCarry = ((((result + GetR8(reg)) & 0xF) - (GetR8(reg) & 0xF)) & 0x10) == 0x10;
}

// Subtract the value at register r8, A and the carry flag. Stored in A.
void CPU::SBC_a_r8(unsigned char reg)
{
	unsigned char result = m_Registers.a - GetR8(reg) - m_FlagRegister.carry;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	//m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	//m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
	m_FlagRegister.halfCarry = ((((result + GetR8(reg) + m_FlagRegister.carry) & 0xF) - (GetR8(reg) + m_FlagRegister.carry & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = ((((result + GetR8(reg) + m_FlagRegister.carry) & 0xF) - (GetR8(reg) - m_FlagRegister.carry & 0xF)) & 0x10) == 0x100 || result == 0;
}

// Bitwise AND between A and register r8.
void CPU::AND_a_r8(unsigned char reg)
{
	m_Registers.a = m_Registers.a & GetR8(reg);

	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = true;
	m_FlagRegister.carry = false;
}

// Bitwise XOR between A and register r8.
void CPU::XOR_a_r8(unsigned char reg)
{
	m_Registers.a = m_Registers.a ^ GetR8(reg);

	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = false;
	m_FlagRegister.carry = false;
}

// Bitwise OR between A and register r8.
void CPU::OR_a_r8(unsigned char reg)
{
	m_Registers.a = m_Registers.a | GetR8(reg);

	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = false;
	m_FlagRegister.carry = false;
}

// Compare the values in A and register r8.
void CPU::CP_a_r8(unsigned char reg)
{
	unsigned char result = m_Registers.a - GetR8(reg);

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = (((m_Registers.a & 0xF) + (GetR8(reg) & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = (((m_Registers.a & 0xF) + (GetR8(reg) & 0xF)) & 0x10) == 0x100;
}

// Add the immediate value and A. Stored in A.
void CPU::ADD_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	unsigned char result = m_Registers.a + immediate;
	m_Registers.a = result;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = ((((result - immediate) & 0xF) + (immediate & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = ((((result - immediate) & 0xF) + (immediate & 0xF)) & 0x10) == 0x100 || result == 0;
}

// Add the immediate value, A and the carry flag. Stored in A.
void CPU::ADC_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	unsigned char result = m_Registers.a + immediate + m_FlagRegister.carry;
	m_Registers.a = result;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = ((((result - immediate - m_FlagRegister.carry) & 0xF) + (immediate + m_FlagRegister.carry & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = ((((result - immediate - m_FlagRegister.carry) & 0xF) + (immediate + m_FlagRegister.carry & 0xF)) & 0x10) == 0x100 || result == 0;
}

// Subtract the immediate value and A. Stored in A.
void CPU::SUB_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	m_FlagRegister.carry = m_Registers.a < immediate;

	unsigned char result = m_Registers.a - immediate;
	m_Registers.a = result;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = ((((result + immediate) & 0xF) - (immediate & 0xF)) & 0x10) == 0x10;
}

// Subtract the immediate value, A and the carry flag. Stored in A.
void CPU::SBC_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	unsigned char result = m_Registers.a - immediate - m_FlagRegister.carry;
	m_Registers.a = result;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = ((((result + immediate + m_FlagRegister.carry) & 0xF) - (immediate - m_FlagRegister.carry & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = ((((result + immediate + m_FlagRegister.carry) & 0xF) - (immediate - m_FlagRegister.carry & 0xF)) & 0x10) == 0x100 || result == 0;
}

// Bitwise AND the immediate value and A. Stored in A.
void CPU::AND_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	m_Registers.a = m_Registers.a & immediate;

	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = true;
	m_FlagRegister.carry = false;
}

// Bitwise XOR the immediate value and A. Stored in A.
void CPU::XOR_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	m_Registers.a = m_Registers.a ^ immediate;

	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = false;
	m_FlagRegister.carry = false;
}

// Bitwise OR the immediate value and A. Stored in A.
void CPU::OR_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	m_Registers.a = m_Registers.a | immediate;

	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = false;
	m_FlagRegister.carry = false;
}

// Compare the immediate value and A.
void CPU::CP_a_imm8()
{
	unsigned char immediate = m_Mem->ReadU8(m_PC++);

	unsigned char result = m_Registers.a - immediate;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = (((m_Registers.a & 0xF) - (immediate & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = (((m_Registers.a & 0xF) + (immediate & 0xF)) & 0x10) == 0x100;
}

// Return conditional.
void CPU::RET_C(unsigned char cond)
{
	unsigned short returnAddress = m_Mem->ReadU16(++m_SP);

	if (cond == 0 && !m_FlagRegister.zero) // Not zero
	{
		m_PC = returnAddress;
		m_SP++;
	}
	else if (cond == 1 && m_FlagRegister.zero) // Zero
	{
		m_PC = returnAddress;
		m_SP++;
	}
	else if (cond == 2 && !m_FlagRegister.carry) // No carry
	{
		m_PC = returnAddress;
		m_SP++;
	}
	else if (cond == 3 && m_FlagRegister.carry) // Carry
	{
		m_PC = returnAddress;
		m_SP++;
	}
	else
	{
		m_SP--;
	}
}

// Return.
void CPU::RET()
{
	m_SP++;
	m_PC = m_Mem->ReadU16(m_SP++); 
}

// Return and enable interrupts.
void CPU::RETI()
{
	m_IME = true;
	RET();
}

// Jump to immediate conditionally.
void CPU::JP_C_imm16(unsigned char cond)
{
	unsigned short jumpAddress = m_Mem->ReadU16(m_PC++);

	if (cond == 0 && !m_FlagRegister.zero) // Not zero
	{
		m_PC = jumpAddress;
	}
	else if (cond == 1 && m_FlagRegister.zero) // Zero
	{
		m_PC = jumpAddress;
	}
	else if (cond == 2 && m_FlagRegister.carry) // No carry
	{
		m_PC = jumpAddress;
	}
	else if (cond == 3 && m_FlagRegister.carry) // Carry
	{
		m_PC = jumpAddress;
	}
	else
	{
		m_PC++;
	}
}

// Jump to immediate.
void CPU::JP_imm16()
{
	unsigned short jumpAddress = m_Mem->ReadU16(m_PC++);
	m_PC = jumpAddress;
}

// Jump to address in HL.
void CPU::JP_HL()
{
	m_PC = GetHL();
}

// Call function conditionally.
void CPU::CALL_C_imm16(unsigned char cond)
{
	if (cond == 0 && !m_FlagRegister.zero) // Not zero
	{
		CALL_imm16();
	}
	else if (cond == 1 && m_FlagRegister.zero) // Zero
	{
		CALL_imm16();
	}
	else if (cond == 2 && m_FlagRegister.carry) // No carry
	{
		CALL_imm16();
	}
	else if (cond == 3 && m_FlagRegister.carry) // Carry
	{
		CALL_imm16();
	}
	else
	{
		m_PC += 2;
	}
}

// Call function.
void CPU::CALL_imm16()
{
	unsigned char jumpAddressLsb = m_Mem->ReadU8(m_PC++);
	unsigned char jumpAddressMsb = m_Mem->ReadU8(m_PC++);

	// Write return address in the stack
	m_SP--;
	m_Mem->WriteU16(m_SP, m_PC);
	m_SP--; // Adjust for the second read
	m_PC = ((unsigned short)jumpAddressMsb << 8) | jumpAddressLsb;
}

// Reset/call function in target vector.
void CPU::RST_tgt3(unsigned char tgt)
{
	// Write return address in the stack
	m_SP--;
	m_Mem->WriteU16(m_SP, ++m_PC);

	m_PC = (unsigned short)tgt;
}

// Pop stack to register r16. (Includes AF)
void CPU::POP_r16(unsigned char reg)
{
	unsigned short value = m_Mem->ReadU16(++m_SP);

	switch (reg)
	{
	case 0:
		SetBC(value);
		break;

	case 1:
		SetDE(value);
		break;

	case 2:
		SetHL(value);
		break;

	case 3:
		SetAF(value);
		break;
	}

	m_SP++;
}

// Push register r16 into stack. (Includes AF)
void CPU::PUSH_r16(unsigned char reg)
{
	m_SP--;

	switch (reg)
	{
	case 0:
		m_Mem->WriteU16(m_SP, GetBC());
		break;

	case 1:
		m_Mem->WriteU16(m_SP, GetDE());
		break;

	case 2:
		m_Mem->WriteU16(m_SP, GetHL());
		break;

	case 3:
		m_Mem->WriteU16(m_SP, GetAF());
		break;
	}
	
	m_SP--;
}

// Load from A into address C + 0xFF00.
void CPU::LDH_c_a()
{
	unsigned short address = m_Registers.c + 0xFF00;

	if (address == 0xFF01) Log::LogInfo((char*)&m_Registers.a);

	m_Mem->WriteU8(address, m_Registers.a);
}

// Load from A into address imm8 + 0xFF00.
void CPU::LDH_imm8_a()
{
	unsigned short address = m_Mem->ReadU8(m_PC++) + 0xFF00;

	if (address == 0xFF01) Log::LogInfo((char*)&m_Registers.a);

	m_Mem->WriteU8(address, m_Registers.a);
}

// Load from A into address imm16.
void CPU::LD_imm16_a()
{
	unsigned char lsb = m_Mem->ReadU8(m_PC++);
	unsigned char msb = m_Mem->ReadU8(m_PC++);

	unsigned short address = ((unsigned short)msb << 8) | lsb;

	if (address == 0xFF01) Log::LogInfo((char*)&m_Registers.a);

	m_Mem->WriteU8(address, m_Registers.a);
}

// Load from address C + 0xFF00 into register A.
void CPU::LDH_a_c()
{
	unsigned short address = m_Registers.c + 0xFF00;
	m_Registers.a = m_Mem->ReadU8(address);
}

// Load from address imm8 + 0xFF00 into register A.
void CPU::LDH_a_imm8()
{
	unsigned short address = m_Mem->ReadU8(m_PC++) + 0xFF00;
	m_Registers.a = m_Mem->ReadU8(address);
}

// Load from address imm16 into register A.
void CPU::LD_a_imm16()
{
	unsigned char lsb = m_Mem->ReadU8(m_PC++);
	unsigned char msb = m_Mem->ReadU8(m_PC++);

	unsigned short address = ((unsigned short)msb << 8) | lsb;
	m_Registers.a = m_Mem->ReadU8(address);
}

// Add to SP a SIGNED immediate.
void CPU::ADD_SP_imm8()
{
	signed char immediate = m_Mem->ReadU8(m_PC++);
	unsigned char result = m_SP + immediate;
	m_SP = result;

	m_FlagRegister.zero = false;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = ((((result - immediate) & 0xF) + (immediate & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = ((((result - immediate) & 0xF) + (immediate & 0xF)) & 0x10) == 0x100;
}

// Add to SP a SIGNED immediate and store it in HL.
void CPU::LD_HL_SLimm8()
{
	signed char immediate = m_Mem->ReadU8(m_PC++);
	unsigned char result = m_SP + immediate;
	SetHL(result);

	m_FlagRegister.zero = false;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = (((m_SP - immediate & 0xF) - (immediate & 0xF)) & 0x10) == 0x10;
	m_FlagRegister.carry = (((m_SP - immediate & 0xF) - (immediate & 0xF)) & 0x10) == 0x100;
}

// Copy register pair HL into SP
void CPU::LD_SP_HL()
{
	m_SP = GetHL();
}

// Disable interrupts
void CPU::DI()
{
	m_IME = false;
}

// Enable interrupts after next instruction
void CPU::EI()
{
	m_EnableIME = true;
}

// Rotate register r8 to the left (circular)
void CPU::RLC_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	// Store the right-most byte and add it later to the shifted number
	unsigned char carryByte = (value & 0b10000000) >> 7;
	value = value << 1;
	value |= carryByte;

	SetR8(reg, value);

	m_FlagRegister.reset();
	m_FlagRegister.zero = value == 0;
	m_FlagRegister.carry = carryByte;
}

// Rotate register r8 to the right (circular)
void CPU::RRC_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	// Store the right-most byte and add it later to the shifted number
	unsigned char carryByte = (value & 0b00000001);
	value = value >> 1;
	value |= (carryByte << 7);

	SetR8(reg, value);

	m_FlagRegister.reset();
	m_FlagRegister.zero = value == 0;
	m_FlagRegister.carry = carryByte;
}

// Rotate register r8 to the left THROUGH the carry flag
void CPU::RL_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	unsigned char oldCarry = m_FlagRegister.carry;
	unsigned char carryByte = (value & 0b10000000);
	value = value >> 1;
	value |= oldCarry;

	SetR8(reg, value);

	m_FlagRegister.reset();
	m_FlagRegister.zero = value == 0;
	m_FlagRegister.carry = carryByte;
}

// Rotate register r8 to the right THROUGH the carry flag
void CPU::RR_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	unsigned char oldCarry = m_FlagRegister.carry;
	unsigned char carryByte = (value & 0b00000001);
	value = value >> 1;
	value |= (oldCarry << 7);

	SetR8(reg, value);

	m_FlagRegister.reset();
	m_FlagRegister.zero = value == 0;
	m_FlagRegister.carry = carryByte;
}

// Shift register r8 to the left (arithmetically)
void CPU::SLA_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	unsigned char carryByte = (value & 0b10000000);
	value = value << 1;

	SetR8(reg, value);

	m_FlagRegister.reset();
	m_FlagRegister.zero = value == 0;
	m_FlagRegister.carry = carryByte;
}

// Shift register r8 to the right (arithmetically)
void CPU::SRA_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	bool upperByte = value & 0b10000000;
	bool carryByte = value & 0b00000001;
	value = value >> 1;
	value |= upperByte << 7;

	SetR8(reg, value);

	m_FlagRegister.reset();
	m_FlagRegister.zero = value == 0;
	m_FlagRegister.carry = carryByte;
}

// Swap the upper 4 bits with the lower 4 bits of register r8
void CPU::SWAP_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	unsigned char lower = value & 0b00001111;
	unsigned char upper = value & 0b11110000;

	value = (lower << 4) | (upper >> 4);
	SetR8(reg, value);
}

// Shift register r8 to the right (logically)
void CPU::SRL_r8(unsigned char reg)
{
	unsigned char value = GetR8(reg);

	bool carryByte = value & 0b00000001;
	value = value >> 1;
	value &= 0b01111111; // Mask out the upper bit (set to 0)

	SetR8(reg, value);

	m_FlagRegister.reset();
	m_FlagRegister.zero = value == 0;
	m_FlagRegister.carry = carryByte;
}

// Test bit b in register r8, set Z if bit is zero
void CPU::BIT(unsigned char bit, unsigned char reg)
{
	unsigned char mask = 0b00000001 << bit;

	bool bitSet = (GetR8(reg) & mask) >> bit;

	m_FlagRegister.zero = !bitSet;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = true;
}

// Set bit b of register r8 to 0
void CPU::RES(unsigned char bit, unsigned char reg)
{
	unsigned char bitToSet = 0b00000001 << bit;

	// Invert the mask to set the bit to 0
	SetR8(reg, GetR8(reg) & ~bitToSet);
}

// Set bit b of register r8 to 1
void CPU::SET(unsigned char bit, unsigned char reg)
{
	unsigned char bitToSet = 0b00000001 << bit;

	// Invert the mask to set the bit to 0
	SetR8(reg, GetR8(reg) | bitToSet);
}
