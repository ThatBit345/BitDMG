#pragma once

/* Get bit from value at position.
*  @param value Value to extract bit from.
*  @param bit Position of the bit to extract (starting at 0).
*  @return Bit at position.
*/
bool GetBit(unsigned char value, int bit);
bool GetBitU16(unsigned short value, int bit);
