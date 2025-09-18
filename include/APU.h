#pragma once

#include <SDL3/SDL.h>

class APU
{
public:
	APU();
	~APU();

	bool IsValid();
	void Tick(int cycles);

private:
	const bool m_SquareWaveforms[4][8] = {
		{0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 1, 1, 1},
		{0, 1, 1, 1, 1, 1, 1, 0}
	};

	bool m_IsValid;

	SDL_AudioStream *m_Stream;
};