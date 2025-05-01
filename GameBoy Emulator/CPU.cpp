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
	unsigned char x = opcode >> 6;
	unsigned char y = (opcode & 0b00111000) >> 3;
	unsigned char z = opcode & 0b00000111;

	unsigned char p = (opcode & 0b00110000) >> 4;
	unsigned char q = (opcode & 0b00001000) >> 3;

	if(x == 0)
	{
		if(z == 0)
		{
			if (y == 0) NOP();
			else if (y == 1) LD_SP();
			else if (y == 2) STOP();
			else if (y == 3) JR_s8();
			else JR_C(y - 4);
		}
		else if(z == 1)
		{
			if (q == 0) LD_RP(p);
			if (q == 1) ADD_HL(p);
		}
	}
	else
	{
		// Print error to console
		std::string errorTxt = "Opcode not found: ";
		std::stringstream str;
		str << std::hex << std::uppercase << (int)opcode;
		errorTxt = errorTxt + str.str();
		Log::LogError(errorTxt.c_str());
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

void CPU::NOP()
{
	return;
}

void CPU::LD_SP() // LD address, SP
{
	unsigned char lsb = this->p_mem->readU8mem(this->pc++);
	unsigned char msb = this->p_mem->readU8mem(this->pc++);

	unsigned short address = ((unsigned short)lsb << 8) | msb;
	this->p_mem->writeU16mem(address, this->sp);
}

void CPU::STOP()
{
	return;
}

void CPU::JR_s8()
{
	unsigned char offset = this->p_mem->readU8mem(this->pc++);
	this->pc += offset;
}

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

void CPU::LD_RP(unsigned char reg)
{
	unsigned short value = this->p_mem->readU16mem(this->pc++);

	switch(reg)
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

void CPU::ADD_HL(unsigned char reg)
{
	unsigned short val = 0x0000;

	switch (reg)
	{
	case 0:
		val = this->GetBC();
		break;

	case 1:
		val = this->GetBC();
		break;

	case 2:
		val = this->GetBC();
		break;

	case 3:
		val = this->sp;
		break;
	}

	unsigned short result = this->GetHL() + val;
	this->SetHL(result);

	// Set flags
	this->flagRegister.subtract = false;
	this->flagRegister.halfCarry = ((result & 0b0000100000000000) >> 11 == 1) ? true : false;
	this->flagRegister.carry = ((result & 0b1000000000000000) >> 15 == 1) ? true : false;
}
