#pragma once

#include <memory>
#include <filesystem>

#include "Memory.h"
#include "Cartridge.h"
#include "CPU.h"

class GameBoy
{
public:
	GameBoy(std::filesystem::path romPath);

	void Update();
	bool IsValid();
	bool IsRunning();

private:
	std::shared_ptr<Memory> m_Memory;
	std::shared_ptr<Cartridge> m_Cartridge;
	CPU m_CPU;

	bool m_Valid;
	bool m_Running;
	int m_CycleCount;
	const int MAX_CYCLES = 69905;

	int m_DividerCycles;
	int m_TimerCycles;
	void HandleTimer(int cycles);
};

