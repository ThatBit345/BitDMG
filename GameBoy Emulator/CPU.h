#pragma once

struct Registers
{
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;
	unsigned char e;
	unsigned char f;
	unsigned char h;
	unsigned char l;

	unsigned short sp;
	unsigned short pc;
};

struct FlagRegister
{
	bool zero;
	bool subtract;
	bool halfCarry;
	bool carry;

	unsigned char toU8()
	{
		unsigned char ret = 0x00;
		ret |= zero << 7;
		ret |= subtract << 6;
		ret |= halfCarry << 5;
		ret |= carry << 4;

		return ret;
	}
};

class CPU
{
private:
	Registers registers;
	FlagRegister flagRegister;

public: 

	CPU();

	// Dual register handling

	void SetAF(unsigned short value);
	void SetBC(unsigned short value);
	void SetDE(unsigned short value);
	void SetHL(unsigned short value);

	unsigned short GetAF();
	unsigned short GetBC();
	unsigned short GetDE();
	unsigned short GetHL();
};