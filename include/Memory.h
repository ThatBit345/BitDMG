#pragma once
#include <array>
#include <memory>

#include "Cartridge.h"

class Memory
{
public:
	Memory(std::shared_ptr<Cartridge> cart);

	/* Get 8-bit value.
	 *  @param address Memory address to read.
	 * @return Value at address.
	 */
	unsigned char ReadU8(unsigned short address);

	/* Get 8-bit value without considering Gameboy state.
	 *  @param address Memory address to read.
	 * @return Value at address.
	 */
	unsigned char ReadU8Unfiltered(unsigned short address);

	/* Write 8-bit value.
	 * @param address Memory address to write.
	 *  @param value Value to write.
	 */
	void WriteU8(unsigned short address, unsigned char value);

	/* Write 8-bit value without considering Gameboy state.
	 * @param address Memory address to write.
	 *  @param value Value to write.
	 */
	void WriteU8Unfiltered(unsigned short address, unsigned char value);

	/* Get 16-bit value.
	 *  @param address Memory address of the first byte to read.
	 *  @return Value at the specified address and the next.
	 */
	unsigned short ReadU16(unsigned short address);

	/* Write 16-bit value.
	 *  @param address Memory address of the first byte to write.
	 * @param value Value to write.
	 */
	void WriteU16(unsigned short address, unsigned short value);

	/* Write 16-bit value.
	 *  @param address Memory address of the first byte to write.
	 *  @param lsb Least significant bit of the value to write.
	 *  @param msb Most significant bit of the value to write.
	 */
	void WriteU16(unsigned short address, unsigned char lsb, unsigned char msb);

	/* Write 16-bit value without considering Gameboy state.
	 *  @param address Memory address of the first byte to write.
	 * @param value Value to write.
	 */
	void WriteU16Unfiltered(unsigned short address, unsigned char value);

	/* Write 16-bit value to the stack.
	 *  @param address Memory address of the first byte to write.
	 * @param value Value to write.
	 */
	void WriteU16Stack(unsigned short address, unsigned short value);

	/* Lock VRAM writes.
	 */
	void LockVRAM();

	/* Unlock VRAM writes.
	 */
	void UnlockVRAM();

	/* Lock OAM writes.
	 */
	void LockOAM();

	/* Unlock OAM writes.
	 */
	void UnlockOAM();

	/* Update the joypad register ($FF00 - P1) with data from m_InputBuffer.
	 */
	void UpdateInputState(bool buffer[8]);

private:
	std::array<unsigned char, 0x10000> m_Memory;

	std::shared_ptr<Cartridge> m_Cartridge;

	bool m_InputBuffer[8];
	void UpdateInputRegister();
	void HandleInputInterrupt(bool isButtons);

	bool m_VramLocked;
	bool m_OamLocked;
};

enum InputButtons
{
	RIGHT = 0,
	LEFT,
	UP,
	DOWN,
	A,
	B,
	SELECT,
	START
};