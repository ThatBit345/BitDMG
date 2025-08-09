#pragma once
#include <array>
#include <memory>

#include "Cartridge.h"

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

enum InterruptType
{
	VBLANK = 0,
	STAT_LCD,
	TIMER,
	SERIAL,
	JOYPAD
};

// Address table for IO registers.
enum IO
{
	// Joypad.
	JOY = 0xFF00,
	// Serial transfer data.
	SB = 0xFF01,
	// Serial transfer control.
	SC = 0xFF02,
	// Divider.
	DIV = 0xff04,
	// Timer counter.
	TIMA = 0xff05,
	// Timer modulo.
	TMA = 0xff06,
	// Timer control.
	TAC = 0xff07,
	// Interrupt flag.
	IF = 0xff0f,
	// Audio channel 1 sweep.
	NR10 = 0xff10,
	// Audio channel 1 length timer & duty cycle.
	NR11 = 0xff11,
	// Audio channel 1 volume & envelope.
	NR12 = 0xff12,
	// Audio channel 1 period low.
	NR13 = 0xff13,
	// Audio channel 1 period high & control.
	NR14 = 0xff14,
	// Audio channel 2 length timer & duty cycle.
	NR21 = 0xff16,
	// Audio channel 2 volume & envelope.
	NR22 = 0xff17,
	// Audio channel 2 period low.
	NR23 = 0xff18,
	// Audio channel 2 period high & control.
	NR24 = 0xff19,
	// Audio channel 3 enable.
	NR30 = 0xff1a,
	// Audio channel 3 length timer.
	NR31 = 0xff1b,
	// Audio channel 3 output level.
	NR32 = 0xff1c,
	// Audio channel 3 period low
	NR33 = 0xff1d,
	// Audio channel 3 period high & control.
	NR34 = 0xff1e,
	// Audio channel 4 length timer.
	NR41 = 0xff20,
	// Audio channel 4 volume & envelope.
	NR42 = 0xff21,
	// Audio channel 4 frequency & randomness.
	NR43 = 0xff22,
	// Audio channel 4 control.
	NR44 = 0xff23,
	// Audio master volume & VIN panning
	NR50 = 0xff24,
	// Audio sound panning
	NR51 = 0xff25,
	// Audio master control
	NR52 = 0xff26,
	// LCD control.
	LCDC = 0xff40,
	// LCD status.
	STAT = 0xff41,
	// Background scroll Y.
	SCY = 0xff42,
	// Background scroll X.
	SCX = 0xff43,
	// Current vertical scanline.
	LY = 0xff44,
	// Vertical scanline compare.
	LYC = 0xff45,
	// DMA Source address & start.
	DMA = 0xff46,
	// Background palette data.
	BGP = 0xff47,
	// Window Y position.
	WY = 0xff4a,
	// Window X position + 7.
	WX = 0xff4b,
	// Interrupt enable.
	IE = 0xffff
};

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

	/* Set the corresponding byte in IF for the needed interrupt.
	* @param interrupt Interrupt type to be requested.
	*/
	void RequestInterrupt(InterruptType interrupt);

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