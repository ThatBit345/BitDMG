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
	unsigned char opcode = this->m_Mem->ReadU8(this->m_PC++);
	
	// Opcode decoding, method described by Scott Mansell in the website below
	// https://archive.gbdev.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
	// and using the reference table described in the pandocs
	// https://gbdev.io/pandocs/CPU_Instruction_Set.html

	unsigned char block = opcode >> 6;
	unsigned char y = (opcode & 0b00111000) >> 3;
	unsigned char z = opcode & 0b00000111;

	unsigned char p = (opcode & 0b00110000) >> 4;
	unsigned char q = (opcode & 0b00001000) >> 3;

	if(block == 0)
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
	else
	{
		UninplementedOpcode(opcode);
	}
	
	return false;
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
	unsigned short value = this->m_Mem->ReadU16(this->m_PC++);

	switch (reg)
	{
	case 0:
		this->SetBC(value);
		break;

	case 1:
		this->SetDE(value);
		break;

	case 2:
		this->SetHL(value);
		break;

	case 3:
		this->m_SP = value;
		break;
	}
}

// Copy the value in register A into the byte pointed to by r16 (r16mem)
void CPU::LD_r16_a(unsigned char reg)
{
	unsigned short address = 0x0000;

	switch (reg)
	{
	case 0:
		address = this->GetBC();
		break;

	case 1:
		address = this->GetDE();
		break;

	case 2:
		address = this->GetHL();
		this->SetHL(this->GetHL() + 1);
		break;

	case 3:
		address = this->GetHL();
		this->SetHL(this->GetHL() - 1);
		break;
	}

	this->m_Mem->WriteU8(address, this->m_Registers.a);
}

// Copy the byte pointed to by register r16 into A (r16mem)
void CPU::LD_a_r16(unsigned char reg)
{
	unsigned short address = 0x0000;

	switch (reg)
	{
	case 0:
		address = this->GetBC();
		break;

	case 1:
		address = this->GetDE();
		break;

	case 2:
		address = this->GetHL();
		this->SetHL(this->GetHL() + 1);
		break;

	case 3:
		address = this->GetHL();
		this->SetHL(this->GetHL() - 1);
		break;
	}

	this->m_Registers.a = this->m_Mem->ReadU8(address);
}

// Copy the immediate value into SP
void CPU::LD_imm16_SP()
{
	unsigned char lsb = this->m_Mem->ReadU8(this->m_PC++);
	unsigned char msb = this->m_Mem->ReadU8(this->m_PC++);

	unsigned short address = ((unsigned short)lsb << 8) | msb;
	this->m_Mem->WriteU16(address, this->m_SP);
}

// Increment value at register r16 by 1
void CPU::INC_r16(unsigned char reg)
{
	switch (reg)
	{
	case 0:
		this->SetBC(this->GetBC() + 1);
		break;

	case 1:
		this->SetDE(this->GetDE() + 1);
		break;

	case 2:
		this->SetHL(this->GetHL() + 1);
		break;

	case 3:
		this->m_SP += 1;
		break;
	}
}

// Decrement value at register r16 by 1
void CPU::DEC_r16(unsigned char reg)
{
	switch (reg)
	{
	case 0:
		this->SetBC(this->GetBC() - 1);
		break;

	case 1:
		this->SetDE(this->GetDE() - 1);
		break;

	case 2:
		this->SetHL(this->GetHL() - 1);
		break;

	case 3:
		this->m_SP -= 1;
		break;
	}
}

// Add value in HL to value at register r16
void CPU::ADD_HL_r16(unsigned char reg)
{
	unsigned short val = 0x0000;

	switch (reg)
	{
	case 0:
		val = this->GetBC();
		break;

	case 1:
		val = this->GetDE();
		break;

	case 2:
		val = this->GetHL();
		break;

	case 3:
		val = this->m_SP;
		break;
	}

	unsigned short result = this->GetHL() + val;
	this->SetHL(result);

	// Set flags
	this->m_FlagRegister.subtract = false;
	this->m_FlagRegister.halfCarry = ((result & 0b0000100000000000) >> 11 == 1);
	this->m_FlagRegister.carry = ((result & 0b1000000000000000) >> 15 == 1);
}

// Increment value at register r8 by 1
void CPU::INC_r8(unsigned char reg)
{
	unsigned char result = 0x00;

	switch(reg)
	{
	case 0:
		this->m_Registers.b += 1;
		result = this->m_Registers.b;
		break;

	case 1:
		this->m_Registers.c += 1;
		result = this->m_Registers.c;
		break;

	case 2:
		this->m_Registers.d += 1;
		result = this->m_Registers.d;
		break;

	case 3:
		this->m_Registers.e += 1;
		result = this->m_Registers.e;
		break;

	case 4:
		this->m_Registers.h += 1;
		result = this->m_Registers.h;
		break;

	case 5:
		this->m_Registers.l += 1;
		result = this->m_Registers.l;
		break;

	case 6:
		result = this->m_Mem->ReadU8(this->GetHL());
		result++;
		this->m_Mem->WriteU8(this->GetHL(), result);
		break;

	case 7:
		this->m_Registers.a += 1;
		result = this->m_Registers.a;
		break;
	}

	this->m_FlagRegister.zero = (result == 0);
	this->m_FlagRegister.subtract = false;
	this->m_FlagRegister.halfCarry = ((result & 0b00001000) >> 3 == 1);
}

// Decrement value at register r8 by 1
void CPU::DEC_r8(unsigned char reg)
{
	unsigned char result = 0x00;

	switch (reg)
	{
	case 0:
		this->m_Registers.b -= 1;
		result = this->m_Registers.b;
		break;

	case 1:
		this->m_Registers.c -= 1;
		result = this->m_Registers.c;
		break;

	case 2:
		this->m_Registers.d -= 1;
		result = this->m_Registers.d;
		break;

	case 3:
		this->m_Registers.e -= 1;
		result = this->m_Registers.e;
		break;

	case 4:
		this->m_Registers.h -= 1;
		result = this->m_Registers.h;
		break;

	case 5:
		this->m_Registers.l -= 1;
		result = this->m_Registers.l;
		break;

	case 6:
		result = this->m_Mem->ReadU8(this->GetHL());
		result--;
		this->m_Mem->WriteU8(this->GetHL(), result);
		break;

	case 7:
		this->m_Registers.a -= 1;
		result = this->m_Registers.a;
		break;
	}

	this->m_FlagRegister.zero = (result == 0);
	this->m_FlagRegister.subtract = true;
	this->m_FlagRegister.halfCarry = ((result & 0b00001000) >> 3 == 1);
}

// Copy the immediate value into register r8
void CPU::LD_r8_imm8(unsigned char reg)
{
	unsigned char value = this->m_Mem->ReadU8(this->m_PC++);

	switch (reg)
	{
	case 0:
		this->m_Registers.b = value;
		break;

	case 1:
		this->m_Registers.c = value;
		break;

	case 2:
		this->m_Registers.d = value;
		break;

	case 3:
		this->m_Registers.e = value;
		break;

	case 4:
		this->m_Registers.h = value;
		break;

	case 5:
		this->m_Registers.l = value;
		break;

	case 6:
		this->m_Mem->WriteU8(this->GetHL(), value);
		break;

	case 7:
		this->m_Registers.a = value;
		break;
	}
}

// Rotate A to the left (circular)
void CPU::RLCA()
{
	// Store the right-most byte and add it later to the shifted number
	unsigned char carryByte = (this->m_Registers.a & 0b10000000) >> 7;
	this->m_Registers.a = this->m_Registers.a << 1;
	this->m_Registers.a |= carryByte;

	this->m_FlagRegister.reset();
	this->m_FlagRegister.zero = this->m_Registers.a == 0;
	this->m_FlagRegister.carry = carryByte;
}

// Rotate A to the right (circular)
void CPU::RRCA()
{
	// Store the left-most byte and add it later to the shifted number
	unsigned char carryByte = (this->m_Registers.a & 0b00000001);
	this->m_Registers.a = this->m_Registers.a >> 1;
	this->m_Registers.a |= (carryByte << 7);

	this->m_FlagRegister.reset();
	this->m_FlagRegister.zero = this->m_Registers.a == 0;
	this->m_FlagRegister.carry = carryByte;
}

// Rotate A to the left THROUGH the carry flag
void CPU::RLA()
{
	unsigned char oldCarry = this->m_FlagRegister.carry;
	unsigned char carryByte = (this->m_Registers.a & 0b10000000);
	this->m_Registers.a = this->m_Registers.a >> 1;
	this->m_Registers.a |= oldCarry;

	this->m_FlagRegister.reset();
	this->m_FlagRegister.zero = this->m_Registers.a == 0;
	this->m_FlagRegister.carry = carryByte;
}

// Rotate A to the right THROUGH the carry flag
void CPU::RRA()
{
	unsigned char oldCarry = this->m_FlagRegister.carry;
	unsigned char carryByte = (this->m_Registers.a & 0b00000001);
	this->m_Registers.a = this->m_Registers.a >> 1;
	this->m_Registers.a |= (oldCarry << 7);

	this->m_FlagRegister.reset();
	this->m_FlagRegister.zero = this->m_Registers.a == 0;
	this->m_FlagRegister.carry = carryByte;
}

// Decimal adjust A to binary coded decimal
void CPU::DAA()
{
	unsigned char adjustment = 0x00;

	if(this->m_FlagRegister.subtract)
	{
		if (this->m_FlagRegister.halfCarry) adjustment += 0x6;
		if (this->m_FlagRegister.carry) adjustment += 0x60;
		this->m_Registers.a -= adjustment;
	}
	else
	{
		if (this->m_FlagRegister.halfCarry || (this->m_Registers.a & 0xF) > 0x9) adjustment += 0x6;
		if (this->m_FlagRegister.carry || this->m_Registers.a > 0x99) adjustment += 0x60;
		this->m_Registers.a += adjustment;
	}

	this->m_FlagRegister.reset();
	this->m_FlagRegister.zero = (this->m_Registers.a == 0);
	this->m_FlagRegister.carry = ((this->m_Registers.a & 0b10000000) >> 7 == 1);
}

// Complement A (bitwise NOT)
void CPU::CPL()
{
	this->m_Registers.a = ~this->m_Registers.a;

	this->m_FlagRegister.subtract = true;
	this->m_FlagRegister.halfCarry = true;
}

// Set the carry flag
void CPU::SCF()
{
	this->m_FlagRegister.subtract = false;
	this->m_FlagRegister.halfCarry = false;
	this->m_FlagRegister.carry = true;
}

// Complement the carry flag
void CPU::CCF()
{
	this->m_FlagRegister.subtract = false;
	this->m_FlagRegister.halfCarry = false;
	this->m_FlagRegister.carry = !this->m_FlagRegister.carry;
}

// Jump offset
void CPU::JR_s8()
{
	unsigned char offset = this->m_Mem->ReadU8(this->m_PC++);
	this->m_PC += offset;
}

// Jump conditional
void CPU::JR_C(unsigned char cond)
{
	unsigned char offset = this->m_Mem->ReadU8(this->m_PC++);

	if(cond == 0 && !this->m_FlagRegister.zero) // Not zero
	{
		this->m_PC += offset;
	}
	else if (cond == 1 && this->m_FlagRegister.zero) // Zero
	{
		this->m_PC += offset;
	}
	else if (cond == 2 && this->m_FlagRegister.carry) // No carry
	{
		this->m_PC += offset;
	}
	else if (cond == 3 && this->m_FlagRegister.carry) // Carry
	{
		this->m_PC += offset;
	}
}

// Enter CPU very low power mode.
void CPU::STOP()
{
	UninplementedOpcode(0x10);
}

void CPU::LD_r8_r8(unsigned char r1, unsigned char r2)
{
	unsigned char value = 0x00;

	switch (r1)
	{
	case 0:
		value = this->m_Registers.b;
		break;

	case 1:
		value = this->m_Registers.c;
		break;

	case 2:
		value = this->m_Registers.d;
		break;

	case 3:
		value = this->m_Registers.e;
		break;

	case 4:
		value = this->m_Registers.h;
		break;

	case 5:
		value = this->m_Registers.l;
		break;

	case 6:
		value = this->m_Mem->ReadU8(this->GetHL());
		break;

	case 7:
		value = this->m_Registers.a;
		break;
	}

	switch (r2)
	{
	case 0:
		this->m_Registers.b = value;
		break;

	case 1:
		this->m_Registers.c = value;
		break;

	case 2:
		this->m_Registers.d = value;
		break;

	case 3:
		this->m_Registers.e = value;
		break;

	case 4:
		this->m_Registers.h = value;
		break;

	case 5:
		this->m_Registers.l = value;
		break;

	case 6:
		this->m_Mem->WriteU8(this->GetHL(), value);
		break;

	case 7:
		this->m_Registers.a = value;
		break;
	}
}

// Enter CPU low-power mode
void CPU::HALT()
{
	UninplementedOpcode(0x76);
}
