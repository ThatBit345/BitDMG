#include "CPU.h"
#include "Log.h"

CPU::CPU()
{
	this->sp = 0xFFFE;
	this->pc = 0x0100;
}

void CPU::Cycle()
{
	
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

