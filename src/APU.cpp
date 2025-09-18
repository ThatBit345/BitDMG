#include "APU.h"

#include "Log.h"

APU::APU() : m_IsValid(true)
{
	SDL_AudioSpec spec;

	spec.channels = 1; // Mono
	spec.format = SDL_AUDIO_F32; // Float
	spec.freq = 8000; // 8000 Hz sample rate

	m_Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	if(!m_Stream)
	{
		Log::LogError("Could not create audio stream");
		m_IsValid = false;
		return;
	}

	SDL_ResumeAudioStreamDevice(m_Stream);
}

APU::~APU()
{
	SDL_DestroyAudioStream(m_Stream);
}

bool APU::IsValid()
{
	return m_IsValid;
}

void APU::Tick(int cycles)
{
	const int minSampleCount = (8000 * sizeof(float)) / 2;
	static int currentSampleTime = 0;

	if(SDL_GetAudioStreamQueued(m_Stream) < minSampleCount)
	{
		static float samples[512];

		for (int i = 0; i < SDL_arraysize(samples); i++)
		{
			const int freq = 100;
			const float phase = currentSampleTime * freq / 8000.0f;
			float sample = SDL_sinf(2 * SDL_PI_F * phase);

			if (sample > 0) samples[i] = 0.2f;
			else if (sample < 0) samples[i] = -0.2f;
			else samples[i] = 0.0f;

			currentSampleTime++;
		}

		currentSampleTime %= 8000;
		SDL_PutAudioStreamData(m_Stream, samples, sizeof(samples));
	}
}
