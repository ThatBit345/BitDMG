#include "CPU.h"
#include "Log.h"

#include <iostream>
#include <sstream>

CPU::CPU(std::shared_ptr<Memory> memory)
{
	// Values after boot sequence
	this->m_SP = 0xFFFE;
	this->m_PC = 0x0101;

	this->m_Mem = memory;
}

bool CPU::Cycle()
{
	unsigned char opcode = m_Mem->ReadU8(m_PC++);
	
	if(!IsValidOpcode(opcode))
	{
		m_PC--;
		return;
	}

	// Opcode decoding, method described by Scott Mansell in the website below
	// https://archive.gbdev.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
	// and using the reference table described in the pandocs
	// https://gbdev.io/pandocs/CPU_Instruction_Set.html

	unsigned char block = opcode >> 6;
	unsigned char y = (opcode & 0b00111000) >> 3;
	unsigned char z = opcode & 0b00000111;

	unsigned char p = (opcode & 0b00110000) >> 4;
	unsigned char q = (opcode & 0b00001000) >> 3;

	if(opcode == 0xCB)
	{
		opcode = m_Mem->ReadU8(m_PC++);

		Log::LogError("CPU: PREFIX FUNCTION");
		UninplementedOpcode(opcode);
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
			else UninplementedOpcode(opcode);
		}
		else if(z == 2)
		{
			if (q == 0) LD_r16_a(p);
			else if (q == 1) LD_a_r16(p);
			else UninplementedOpcode(opcode);
		}
		else if(z == 3)
		{
			if (q == 0) INC_r16(p);
			else if (q == 1) DEC_r16(p);
			else UninplementedOpcode(opcode);
		}
		else if(z == 4)
		{
			INC_r8(y);
		}
		else if(z == 5)
		{
			DEC_r8(y);
		}
		else if(z == 6)
		{
			LD_r8_imm8(y);
		}
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
			else UninplementedOpcode(opcode);
		}
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
		else UninplementedOpcode(opcode);
	}
	else if (block == 3)
	{
		
	}
	else
	{
		UninplementedOpcode(opcode);
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
}

void CPU::SetR16(unsigned char reg, unsigned char value)
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
	return opcode == 0xD3 ||
		opcode == 0xDB ||
		opcode == 0xDD ||
		opcode == 0xE3 ||
		opcode == 0xE4 ||
		opcode == 0xEB ||
		opcode == 0xEC ||
		opcode == 0xED ||
		opcode == 0xF4 ||
		opcode == 0xFC ||
		opcode == 0xFD;
}

void CPU::UninplementedOpcode(int opcode)
{
	// Print error to console
	std::string errorTxt = "Opcode not found: ";
	std::stringstream str;
	str << std::hex << std::uppercase << (int)opcode;
	errorTxt = errorTxt + str.str();
	Log::LogError(errorTxt.c_str());
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
	m_FlagRegister.halfCarry = ((result & 0b0000100000000000) >> 11 == 1);
	m_FlagRegister.carry = ((result & 0b1000000000000000) >> 15 == 1);
}

// Increment value at register r8 by 1
void CPU::INC_r8(unsigned char reg)
{
	unsigned char result = GetR8(reg) + 1;
	SetR8(reg, result);

	m_FlagRegister.zero = (result == 0);
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = ((result & 0b00001000) >> 3 == 1);
}

// Decrement value at register r8 by 1
void CPU::DEC_r8(unsigned char reg)
{
	unsigned char result = GetR8(reg) - 1;
	SetR8(reg, result);

	m_FlagRegister.zero = (result == 0);
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = ((result & 0b00001000) >> 3 == 1);
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
	m_FlagRegister.zero = m_Registers.a == 0;
	m_FlagRegister.carry = carryByte;
}

// Decimal adjust A to binary coded decimal
void CPU::DAA()
{
	unsigned char adjustment = 0x00;

	if(m_FlagRegister.subtract)
	{
		if (m_FlagRegister.halfCarry) adjustment += 0x6;
		if (m_FlagRegister.carry) adjustment += 0x60;
		m_Registers.a -= adjustment;
	}
	else
	{
		if (m_FlagRegister.halfCarry || (m_Registers.a & 0xF) > 0x9) adjustment += 0x6;
		if (m_FlagRegister.carry || m_Registers.a > 0x99) adjustment += 0x60;
		m_Registers.a += adjustment;
	}

	m_FlagRegister.reset();
	m_FlagRegister.zero = (m_Registers.a == 0);
	m_FlagRegister.carry = ((m_Registers.a & 0b10000000) >> 7 == 1);
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
	unsigned char offset = m_Mem->ReadU8(m_PC++);
	m_PC += offset;
}

// Jump conditional
void CPU::JR_C(unsigned char cond)
{
	unsigned char offset = m_Mem->ReadU8(m_PC++);

	if(cond == 0 && !m_FlagRegister.zero) // Not zero
	{
		m_PC += offset;
	}
	else if (cond == 1 && m_FlagRegister.zero) // Zero
	{
		m_PC += offset;
	}
	else if (cond == 2 && m_FlagRegister.carry) // No carry
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
	SetR8(r2, GetR8(r1));
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
	m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
}

// Add the value at register r8, A and the carry flag. Stored in A.
void CPU::ADC_a_r8(unsigned char reg)
{
	unsigned char result = m_Registers.a + GetR8(reg) + m_FlagRegister.carry;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = false;
	m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
}

// Subtract the value at register r8 from A. Stored in A.
void CPU::SUB_a_r8(unsigned char reg)
{
	unsigned char result = m_Registers.a - GetR8(reg);

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
}

// Subtract the value at register r8, A and the carry flag. Stored in A.
void CPU::SBC_a_r8(unsigned char reg)
{
	unsigned char result = m_Registers.a - GetR8(reg) - m_FlagRegister.carry;

	m_FlagRegister.zero = result == 0;
	m_FlagRegister.subtract = true;
	m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
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
	m_FlagRegister.halfCarry = ((result & 0x00001000) >> 3) == 0;
	m_FlagRegister.carry = ((result & 0x10000000) >> 7) == 0;
}