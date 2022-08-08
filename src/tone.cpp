#include "tone.hpp"
#include <chrono>
#include <fmt/core.h>

VIPR_Emulator::ToneGenerator::ToneGenerator() : processing(false), generate_tone(false), current_period(0.0)
{
}

VIPR_Emulator::ToneGenerator::~ToneGenerator()
{
	if (processing)
	{
		processing = false;
		AudioProcessingThread.join();
	}
	SDL_PauseAudioDevice(device, 1);
	SDL_CloseAudioDevice(device);
}

void VIPR_Emulator::ToneGenerator::SetupToneGenerator()
{
	processing = true;
	SDL_AudioSpec desired;
	SDL_zero(desired);
	desired.freq = 192000;
	desired.channels = 1;
	desired.samples = 4096;
	desired.format = AUDIO_S32;
	device = SDL_OpenAudioDevice(nullptr, 0, &desired, &spec, 0);
	SDL_PauseAudioDevice(device, 0);
	AudioProcessingThread = std::thread(ToneGenerator::AudioProcessor, this);
}

void VIPR_Emulator::ToneGenerator::AudioProcessor(ToneGenerator *generator)
{
	std::chrono::high_resolution_clock::time_point audio_tp = std::chrono::high_resolution_clock::now();
	double audio_accumulator = 0.0;
	while (generator->processing)
	{
		AudioFrame current_frame;
		for (size_t i = 0; i < current_frame.size(); )
		{
			bool idle = true;
			std::chrono::high_resolution_clock::time_point current_tp = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> delta_time = current_tp - audio_tp;
			if (delta_time.count() > 0.25)
			{
				delta_time = std::chrono::duration<double>(0.25);
			}
			audio_accumulator += delta_time.count();
			audio_tp = current_tp;
			while (audio_accumulator >= 1.0 / static_cast<double>(generator->spec.freq) && i < current_frame.size())
			{
				if (idle)
				{
					idle = false;
				}
				audio_accumulator -= 1.0 / static_cast<double>(generator->spec.freq);
				double value = 0.0;
				if (generator->generate_tone)
				{
					value = 0.2 * ((generator->current_period < 0.5 / 1400.0) ? static_cast<double>(INT32_MAX) : static_cast<double>(INT32_MIN));
				}
				current_frame[i] = static_cast<int>(value);
				generator->current_period += 1.0 / static_cast<double>(generator->spec.freq);
				if (generator->current_period >= 1.0 / 1400.0)
				{
					generator->current_period -= 1.0 / 1400.0;
				}
				++i;
			}
			if (idle)
			{
				SDL_Delay(1);
			}
		}
		constexpr size_t buffer_max = current_frame.size() * sizeof(int) * 2;
		while (SDL_GetQueuedAudioSize(generator->device) >= buffer_max)
		{
			SDL_Delay(10);
			audio_tp = std::chrono::high_resolution_clock::now();
		}
		SDL_QueueAudio(generator->device, current_frame.data(), current_frame.size() * sizeof(int));
	}
}
