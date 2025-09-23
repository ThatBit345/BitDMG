#include "GameBoy.h"

#include <filesystem>
#include <SDL3/SDL.h>

#include "Log.h"

GameBoy::GameBoy(std::filesystem::path romPath, SDL_Window *window) : m_CPU(nullptr), m_PPU(nullptr), m_Valid(true), m_Running(true), m_CycleCount(0), m_DividerCycles(0), m_TimerCycles(0)
{
	Log::LogInfo("BitDMG v0.7.1");

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

	std::string title = "BitDMG - " + m_Cartridge->GetCartName();
	SDL_SetWindowTitle(window, title.c_str());

	m_Window = window;

	m_Memory = std::make_shared<Memory>(m_Cartridge);
	m_CPU = (m_Memory);
	m_PPU = (m_Memory);
	m_PPU.ConfigureLCD(window);

	for (size_t i = 0; i < 8; i++)
	{
		m_InputBuffer[i] = false;
	}

	Log::LogInfo("Emulator started succesfully!");
}

void GameBoy::Update()
{
	SDL_Time startTime;
	SDL_GetCurrentTime(&startTime);

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_WINDOW_RESIZED)
		{
			m_PPU.ConfigureLCD(m_Window);
		}
		else if (event.type == SDL_EVENT_QUIT)
		{
			m_Running = false;
			return;
		}
		else if (event.type == SDL_EVENT_KEY_DOWN)
		{
			if (event.key.scancode == SDL_SCANCODE_UP)
			{
				m_InputBuffer[InputButtons::UP] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_DOWN)
			{
				m_InputBuffer[InputButtons::DOWN] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_LEFT)
			{
				m_InputBuffer[InputButtons::LEFT] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_RIGHT)
			{
				m_InputBuffer[InputButtons::RIGHT] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_X)
			{
				m_InputBuffer[InputButtons::A] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_Z)
			{
				m_InputBuffer[InputButtons::B] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_RETURN)
			{
				m_InputBuffer[InputButtons::START] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_BACKSPACE)
			{
				m_InputBuffer[InputButtons::SELECT] = true;
			}
			else if (event.key.scancode == SDL_SCANCODE_1)
			{
                m_PPU.SetLCDPalette(0);
			}
			else if (event.key.scancode == SDL_SCANCODE_2)
			{
                m_PPU.SetLCDPalette(1);
			}
			else if (event.key.scancode == SDL_SCANCODE_3)
			{
                m_PPU.SetLCDPalette(2);
			}
			else if (event.key.scancode == SDL_SCANCODE_4)
			{
                m_PPU.SetLCDPalette(3);
			}
		}
		else if (event.type == SDL_EVENT_KEY_UP)
		{
			if (event.key.scancode == SDL_SCANCODE_UP)
			{
				m_InputBuffer[InputButtons::UP] = false;
			}
			else if (event.key.scancode == SDL_SCANCODE_DOWN)
			{
				m_InputBuffer[InputButtons::DOWN] = false;
			}
			else if (event.key.scancode == SDL_SCANCODE_LEFT)
			{
				m_InputBuffer[InputButtons::LEFT] = false;
			}
			else if (event.key.scancode == SDL_SCANCODE_RIGHT)
			{
				m_InputBuffer[InputButtons::RIGHT] = false;
			}
			else if (event.key.scancode == SDL_SCANCODE_X)
			{
				m_InputBuffer[InputButtons::A] = false;
			}
			else if (event.key.scancode == SDL_SCANCODE_Z)
			{
				m_InputBuffer[InputButtons::B] = false;
			}
			else if (event.key.scancode == SDL_SCANCODE_RETURN)
			{
				m_InputBuffer[InputButtons::START] = false;
			}
			else if (event.key.scancode == SDL_SCANCODE_BACKSPACE)
			{
				m_InputBuffer[InputButtons::SELECT] = false;
			}
		}
	}
	m_Memory->UpdateInputState(m_InputBuffer);

	while (m_CycleCount < MAX_CYCLES)
	{
		int cycles = m_CPU.Cycle();
		m_CycleCount += cycles * 4; // Transform M-Cycles to Clock Cycles
		m_Running = cycles != -1;

		m_PPU.Tick(cycles * 4);

		HandleTimer(cycles);
	}

	m_PPU.Render();
	m_CycleCount = 0;

	// Limit FPS to ~60 (GameBoy runs slightly slower than 60 FPS)
	SDL_Time endTime;
	SDL_GetCurrentTime(&endTime);

	SDL_Time elapsed = endTime - startTime;
	SDL_Time frameDiff = 17400000 - elapsed;

	if (frameDiff > 0)
	{
		SDL_Delay(frameDiff / 1000000);
	}
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
	if (m_DividerCycles >= 64)
	{
		m_Memory->WriteU8Unfiltered(IO::DIV, m_Memory->ReadU8(IO::DIV) + 1);
	}

	m_TimerCycles += mCycles;
	unsigned char TAC = m_Memory->ReadU8(IO::TAC);
	if ((TAC & 0x4) >> 2) // If timer enabled
	{
		int freq = 256;

		switch (TAC & 0x3)
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

		if (m_TimerCycles >= freq)
		{
			unsigned char TIMA = m_Memory->ReadU8(IO::TIMA);

			if (TIMA == 0xFF) // Timer overflow
			{
				m_Memory->WriteU8Unfiltered(IO::TIMA, m_Memory->ReadU8(IO::TMA));		   // Reset to what TMA especifies
				m_Memory->RequestInterrupt(InterruptType::TIMER);
			}
			else
				m_Memory->WriteU8Unfiltered(IO::TIMA, TIMA + 1);

			m_TimerCycles -= freq;
		}
	}
}
