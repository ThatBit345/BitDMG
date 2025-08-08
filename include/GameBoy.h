#pragma once

#include <memory>
#include <filesystem>

#include "Memory.h"
#include "Cartridge.h"
#include "CPU.h"
#include "PPU.h"

class GameBoy
{
public:
	GameBoy(std::filesystem::path romPath, SDL_Window *window);

	/* Execute a frame of the GameBoy game.
	 */
	void Update();

	/* Check that the GameBoy has all required components to run.
	 * @return True if the GameBoy can run correctly.
	 */
	bool IsValid();

	/* Check if the GameBoy emulator is running (CPU didn't encounter any errors).
	 * @return True if the GameBoy emulator is running correctly.
	 */
	bool IsRunning();

private:
	const int MAX_CYCLES = 69905;
	
	std::shared_ptr<Memory> m_Memory;
	std::shared_ptr<Cartridge> m_Cartridge;
	CPU m_CPU;
	PPU m_PPU;
	SDL_Window *m_Window;

	bool m_Valid;
	bool m_Running;
	int m_CycleCount;

	bool m_InputBuffer[8];

	int m_DividerCycles;
	int m_TimerCycles;

	/* Handle DIV and TIMA timers and request their interrupts.
	 * @param mCycles CPU M-Cycles taken during last operation.
	 */
	void HandleTimer(int cycles);
};
