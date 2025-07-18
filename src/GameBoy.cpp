#include "GameBoy.h"

#include <filesystem>
#include <SDL3/SDL.h>

#include "Log.h"

GameBoy::GameBoy(std::filesystem::path romPath, SDL_Window* window) : m_CPU(nullptr), m_PPU(nullptr), m_Valid(true), m_Running(true), m_CycleCount(0), m_DividerCycles(0), m_TimerCycles(0)
{
    Log::LogInfo("BitBoy v0.1.0");

    if (romPath.empty())
    {
        Log::LogError("ROM file not found!");
        m_Valid = false;
        return;
    }

    m_Cartridge = std::make_shared<Cartridge>(romPath.string().c_str());

    if (!m_Cartridge->IsValid())
    {
        Log::LogError("Error loading ROM file!");
        m_Valid = false;
        return;
    }

    m_Memory = std::make_shared<Memory>(m_Cartridge);
    m_CPU = (m_Memory);
    m_PPU = (m_Memory);
	m_PPU.ConfigureLCD(window);

    Log::LogInfo("Emulator started succesfully!");
}

/* Execute the Gameboy game for one frame.
*/
void GameBoy::Update()
{
	SDL_Time startTime;
	SDL_GetCurrentTime(&startTime);

    while(m_CycleCount < MAX_CYCLES)
    {
        int cycles = m_CPU.Cycle();
        m_CycleCount += cycles * 4; // Transform M-Cycles to Clock Cycles
        m_Running = cycles != -1;

        m_PPU.Tick(cycles * 4);

        HandleTimer(cycles);
    }

	m_PPU.Render();
    m_CycleCount = 0;

	// Limit FPS to ~60 (Gameboy runs slightly slower than 60Hz)
	SDL_Time endTime;
	SDL_GetCurrentTime(&endTime);

	SDL_Time elapsed = endTime - startTime;
	SDL_Time frameDiff = 17400000 - elapsed;

	if(frameDiff > 0) 
	{
		SDL_Delay(frameDiff / 1000000);
	}
}

/* Check that the Gameboy has all required components to run.
*/
bool GameBoy::IsValid()
{
	return m_Valid;
}

/* Return if the Gameboy emulator is running (CPU didn't encounter any errors).
*/
bool GameBoy::IsRunning()
{
    return m_Running;
}

/* Handle DIV and TIMA timers and request their interrupts.
*  [mCycles] -> CPU M-Cycles taken during last operation
*/
void GameBoy::HandleTimer(int mCycles)
{
    // Divider
    m_DividerCycles += mCycles;
    if(m_DividerCycles >= 64)
    {
        m_Memory->WriteU8Unfiltered(0xFF04, m_Memory->ReadU8(0xFF04) + 1);
    }

    m_TimerCycles += mCycles;
    unsigned char TAC = m_Memory->ReadU8(0xFF07);
    if ((TAC & 0x4) >> 2) // If timer enabled
    {
        int freq = 256;
        
        switch(TAC & 0x3)
        {
        case 0b00: 
            freq = 256;
            break;

        case 0b01:
            freq = 4;
            break;

        case 0b10:
            freq = 16;
            break;

        case 0b11:
            freq = 64;
            break;
        }

        if(m_TimerCycles >= freq)
        {
            unsigned char TIMA = m_Memory->ReadU8(0xFF05);

            if(TIMA == 0xFF) // Timer overflow
            {
                m_Memory->WriteU8Unfiltered(0xFF05, m_Memory->ReadU8(0xFF06));       // Reset to what TMA especifies
                m_Memory->WriteU8Unfiltered(0xFF0F, m_Memory->ReadU8(0xFF0F) | 0b100); // Request interrupt
            }
            else m_Memory->WriteU8Unfiltered(0xFF05, TIMA + 1);

            m_TimerCycles -= freq;
        }
    }
}
