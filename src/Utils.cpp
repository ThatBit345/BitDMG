#include "Utils.h"

bool GetBit(unsigned char value, int bit)
{
	return (value >> bit) & 0b1;
}

bool GetBitU16(unsigned short value, int bit)
{
    return (value >> bit) & 0b1;
}
