#include "CPU.h"
#include "Log.h"

#include <iostream>
#include <sstream>

CPU::CPU(std::shared_ptr<Memory> memory)
{
	// Values after boot sequence
	this->sp = 0xFFFE;
	this->pc = 0x0101;

	this->p_mem = memory;
}

bool CPU::Cycle()
{
	unsigned char opcode = this->p_mem->readU8mem(this->pc++);
	
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
	this->registers.a = value >> 8;
	this->flagRegister.fromU8(value);
}

void CPU::SetBC(unsigned short value)
{
	this->registers.b = value >> 8;
	this->registers.c = value;
}

void CPU::SetDE(unsigned short value)
{
	this->registers.d = value >> 8;
	this->registers.e = value;
}

void CPU::SetHL(unsigned short value)
{
	this->registers.h = value >> 8;
	this->registers.l = value;
}

unsigned short CPU::GetAF()
{
	return ((unsigned short)this->registers.a << 8) | this->flagRegister.toU8();
}

unsigned short CPU::GetBC()
{
	return ((unsigned short)this->registers.b << 8) | this->registers.c;
}

unsigned short CPU::GetDE()
{
	return ((unsigned short)this->registers.d << 8) | this->registers.e;
}

unsigned short CPU::GetHL()
{
	return ((unsigned short)this->registers.h << 8) | this->registers.l;
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
	unsigned short value = this->p_mem->readU16mem(this->pc++);

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
		this->sp = value;
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

	this->p_mem->writeU8mem(address, this->registers.a);
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

	this->registers.a = this->p_mem->readU8mem(address);
}

// Copy the immediate value into SP
void CPU::LD_imm16_SP()
{
	unsigned char lsb = this->p_mem->readU8mem(this->pc++);
	unsigned char msb = this->p_mem->readU8mem(this->pc++);

	unsigned short address = ((unsigned short)lsb << 8) | msb;
	this->p_mem->writeU16mem(address, this->sp);
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
		this->sp += 1;
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
		this->sp -= 1;
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
		val = this->sp;
		break;
	}

	unsigned short result = this->GetHL() + val;
	this->SetHL(result);

	// Set flags
	this->flagRegister.subtract = false;
	this->flagRegister.halfCarry = ((result & 0b0000100000000000) >> 11 == 1);
	this->flagRegister.carry = ((result & 0b1000000000000000) >> 15 == 1);
}

// Increment value at register r8 by 1
void CPU::INC_r8(unsigned char reg)
{
	unsigned char result = 0x00;

	switch(reg)
	{
	case 0:
		this->registers.b += 1;
		result = this->registers.b;
		break;

	case 1:
		this->registers.c += 1;
		result = this->registers.c;
		break;

	case 2:
		this->registers.d += 1;
		result = this->registers.d;
		break;

	case 3:
		this->registers.e += 1;
		result = this->registers.e;
		break;

	case 4:
		this->registers.h += 1;
		result = this->registers.h;
		break;

	case 5:
		this->registers.l += 1;
		result = this->registers.l;
		break;

	case 6:
		result = this->p_mem->readU8mem(this->GetHL());
		result++;
		this->p_mem->writeU8mem(this->GetHL(), result);
		break;

	case 7:
		this->registers.a += 1;
		result = this->registers.a;
		break;
	}

	this->flagRegister.zero = (result == 0);
	this->flagRegister.subtract = false;
	this->flagRegister.halfCarry = ((result & 0b00001000) >> 3 == 1);
}

// Decrement value at register r8 by 1
void CPU::DEC_r8(unsigned char reg)
{
	unsigned char result = 0x00;

	switch (reg)
	{
	case 0:
		this->registers.b -= 1;
		result = this->registers.b;
		break;

	case 1:
		this->registers.c -= 1;
		result = this->registers.c;
		break;

	case 2:
		this->registers.d -= 1;
		result = this->registers.d;
		break;

	case 3:
		this->registers.e -= 1;
		result = this->registers.e;
		break;

	case 4:
		this->registers.h -= 1;
		result = this->registers.h;
		break;

	case 5:
		this->registers.l -= 1;
		result = this->registers.l;
		break;

	case 6:
		result = this->p_mem->readU8mem(this->GetHL());
		result--;
		this->p_mem->writeU8mem(this->GetHL(), result);
		break;

	case 7:
		this->registers.a -= 1;
		result = this->registers.a;
		break;
	}

	this->flagRegister.zero = (result == 0);
	this->flagRegister.subtract = true;
	this->flagRegister.halfCarry = ((result & 0b00001000) >> 3 == 1);
}

// Copy the immediate value into register r8
void CPU::LD_r8_imm8(unsigned char reg)
{
	unsigned char value = this->p_mem->readU8mem(this->pc++);

	switch (reg)
	{
	case 0:
		this->registers.b = value;
		break;

	case 1:
		this->registers.c = value;
		break;

	case 2:
		this->registers.d = value;
		break;

	case 3:
		this->registers.e = value;
		break;

	case 4:
		this->registers.h = value;
		break;

	case 5:
		this->registers.l = value;
		break;

	case 6:
		this->p_mem->writeU8mem(this->GetHL(), value);
		break;

	case 7:
		this->registers.a = value;
		break;
	}
}

// Rotate A to the left (circular)
void CPU::RLCA()
{
	// Store the right-most byte and add it later to the shifted number
	unsigned char carryByte = (this->registers.a & 0b10000000) >> 7;
	this->registers.a = this->registers.a << 1;
	this->registers.a |= carryByte;

	this->flagRegister.reset();
	this->flagRegister.zero = this->registers.a == 0;
	this->flagRegister.carry = carryByte;
}

// Rotate A to the right (circular)
void CPU::RRCA()
{
	// Store the left-most byte and add it later to the shifted number
	unsigned char carryByte = (this->registers.a & 0b00000001);
	this->registers.a = this->registers.a >> 1;
	this->registers.a |= (carryByte << 7);

	this->flagRegister.reset();
	this->flagRegister.zero = this->registers.a == 0;
	this->flagRegister.carry = carryByte;
}

// Rotate A to the left THROUGH the carry flag
void CPU::RLA()
{
	unsigned char oldCarry = this->flagRegister.carry;
	unsigned char carryByte = (this->registers.a & 0b10000000);
	this->registers.a = this->registers.a >> 1;
	this->registers.a |= oldCarry;

	this->flagRegister.reset();
	this->flagRegister.zero = this->registers.a == 0;
	this->flagRegister.carry = carryByte;
}

// Rotate A to the right THROUGH the carry flag
void CPU::RRA()
{
	unsigned char oldCarry = this->flagRegister.carry;
	unsigned char carryByte = (this->registers.a & 0b00000001);
	this->registers.a = this->registers.a >> 1;
	this->registers.a |= (oldCarry << 7);

	this->flagRegister.reset();
	this->flagRegister.zero = this->registers.a == 0;
	this->flagRegister.carry = carryByte;
}

// Decimal adjust A to binary coded decimal
void CPU::DAA()
{
	unsigned char adjustment = 0x00;

	if(this->flagRegister.subtract)
	{
		if (this->flagRegister.halfCarry) adjustment += 0x6;
		if (this->flagRegister.carry) adjustment += 0x60;
		this->registers.a -= adjustment;
	}
	else
	{
		if (this->flagRegister.halfCarry || (this->registers.a & 0xF) > 0x9) adjustment += 0x6;
		if (this->flagRegister.carry || this->registers.a > 0x99) adjustment += 0x60;
		this->registers.a += adjustment;
	}

	this->flagRegister.reset();
	this->flagRegister.zero = (this->registers.a == 0);
	this->flagRegister.carry = ((this->registers.a & 0b10000000) >> 7 == 1);
}

// Complement A (bitwise NOT)
void CPU::CPL()
{
	this->registers.a = ~this->registers.a;

	this->flagRegister.subtract = true;
	this->flagRegister.halfCarry = true;
}

// Set the carry flag
void CPU::SCF()
{
	this->flagRegister.subtract = false;
	this->flagRegister.halfCarry = false;
	this->flagRegister.carry = true;
}

// Complement the carry flag
void CPU::CCF()
{
	this->flagRegister.subtract = false;
	this->flagRegister.halfCarry = false;
	this->flagRegister.carry = !this->flagRegister.carry;
}

// Jump offset
void CPU::JR_s8()
{
	unsigned char offset = this->p_mem->readU8mem(this->pc++);
	this->pc += offset;
}

// Jump conditional
void CPU::JR_C(unsigned char cond)
{
	unsigned char offset = this->p_mem->readU8mem(this->pc++);

	if(cond == 0 && !this->flagRegister.zero) // Not zero
	{
		this->pc += offset;
	}
	else if (cond == 1 && this->flagRegister.zero) // Zero
	{
		this->pc += offset;
	}
	else if (cond == 2 && this->flagRegister.carry) // No carry
	{
		this->pc += offset;
	}
	else if (cond == 3 && this->flagRegister.carry) // Carry
	{
		this->pc += offset;
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
		value = this->registers.b;
		break;

	case 1:
		value = this->registers.c;
		break;

	case 2:
		value = this->registers.d;
		break;

	case 3:
		value = this->registers.e;
		break;

	case 4:
		value = this->registers.h;
		break;

	case 5:
		value = this->registers.l;
		break;

	case 6:
		value = this->p_mem->readU8mem(this->GetHL());
		break;

	case 7:
		value = this->registers.a;
		break;
	}

	switch (r2)
	{
	case 0:
		this->registers.b = value;
		break;

	case 1:
		this->registers.c = value;
		break;

	case 2:
		this->registers.d = value;
		break;

	case 3:
		this->registers.e = value;
		break;

	case 4:
		this->registers.h = value;
		break;

	case 5:
		this->registers.l = value;
		break;

	case 6:
		this->p_mem->writeU8mem(this->GetHL(), value);
		break;

	case 7:
		this->registers.a = value;
		break;
	}
}

// Enter CPU low-power mode
void CPU::HALT()
{
	UninplementedOpcode(0x76);
}
