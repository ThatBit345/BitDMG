#include "GameBoy.h"

#include <filesystem>

#include "Log.h"

GameBoy::GameBoy(std::filesystem::path romPath) : m_CPU(nullptr), m_Valid(true), m_Running(true), m_CycleCount(0), m_DividerCycles(0), m_TimerCycles(0)
{
    if (romPath.empty())
    {
        Log::LogError("File not found!");
        m_Valid = false;
    }

    m_Cartridge = std::make_shared<Cartridge>(romPath.string().c_str());

    if (!m_Cartridge->IsValid())
    {
        Log::LogError("Error loading ROM file!");
        m_Valid = false;
    }

    m_Memory = std::make_shared<Memory>(m_Cartridge);
    m_CPU = (m_Memory);

    Log::LogInfo("Emulator started succesfully!");
}

void GameBoy::Update()
{
    m_CPU.Cycle();
    //while(m_CycleCount < MAX_CYCLES)
    //{
    //    int cycles = m_CPU.Cycle();
    //    m_CycleCount += cycles * 4; // Transform M-Cycles to Clock Cycles
    //    m_Running = cycles == -1;
    //
    //    HandleTimer(cycles);
    //}
    //
    // DRAW FRAME

    m_CycleCount = 0;
}

bool GameBoy::IsValid()
{
	return m_Valid;
}

bool GameBoy::IsRunning()
{
    return m_Running;
}

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
    if ((TAC >> 2) & 0x4) // If timer enabled
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
                m_Memory->WriteU8Unfiltered(0xFF0F, m_Memory->ReadU8(0xFF0F) | 0x4); // Request interrupt
            }
            else m_Memory->WriteU8Unfiltered(0xFF05, TIMA + 1);

            m_TimerCycles -= freq;
        }
    }
}
