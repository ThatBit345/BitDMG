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

	if(x == 0)
	{
		if(z == 0)
		{
			if (y == 0) NOP();
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
	this->registers.f = value;
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
	return ((unsigned short)this->registers.a << 8) | this->registers.f;
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
	
}

void CPU::LD_SP() // LD address, SP
{
	unsigned char lsb = this->p_mem->readU8mem(this->pc++);
	unsigned char msb = this->p_mem->readU8mem(this->pc++);

	unsigned short address = ((unsigned short)lsb << 8) | msb;
	this->p_mem->writeU16mem(address, this->sp);
}
