#include "vp595.hpp"
#include "audio.hpp"
#include <chrono>
#include <fmt/core.h>

VIPR_Emulator::VP595::VP595(double input_frequency) : /* processing(false) , */ pause(false), generate_tone(false), volume(0.5), current_period(0.0), frequency_generator(input_frequency, CDP1863::InputClockType::Clock1)
{
	SetFrequency(0x00);
}

VIPR_Emulator::VP595::~VP595()
{
	/*
	if (processing)
	{
		processing = false;
		AudioProcessingThread.join();
	}
	SDL_PauseAudioDevice(device, 1);
	SDL_CloseAudioDevice(device);
	*/
}

/*
void VIPR_Emulator::VP595::SetupVP595(std::string output_audio_device)
{
	if (processing)
	{
		processing = false;
		AudioProcessingThread.join();
		SDL_PauseAudioDevice(device, 1);
		SDL_CloseAudioDevice(device);
	}
	processing = true;
	SDL_AudioSpec desired;
	SDL_zero(desired);
	desired.freq = 192000;
	desired.channels = 1;
	desired.samples= 4096;
	desired.format = AUDIO_S32;
	device = SDL_OpenAudioDevice(output_audio_device.c_str(), 0, &desired, &spec, 0);
	SDL_PauseAudioDevice(device, 0);
	AudioProcessingThread = std::thread(VP595::AudioProcessor, this);
}

void VIPR_Emulator::VP595::AudioProcessor(VP595 *generator)
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
				double frequency = generator->frequency_generator.GetOutputFrequency();
				double value = 0.0;
				if (generator->generate_tone && !generator->pause)
				{
					value = generator->volume * 0.4 * ((generator->current_period < 0.5 / frequency) ? static_cast<double>(INT32_MAX) : static_cast<double>(INT32_MIN));
				}
				current_frame[i] = static_cast<int>(value);
				generator->current_period += 1.0 / static_cast<double>(generator->spec.freq);
				if (generator->current_period >= 1.0 / frequency)
				{
					generator->current_period -= 1.0 / frequency;
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
*/

double VIPR_Emulator::VP595_audio_output_callback(void *source)
{
	constexpr double sample_rate = 1.0 / 192000.0;
	VP595 *simple_sound_board = static_cast<VP595 *>(source);
	double frequency = simple_sound_board->frequency_generator.GetOutputFrequency();
	double value = 0.0;
	if (simple_sound_board->generate_tone && !simple_sound_board->pause)
	{
		value = 0.4 * ((simple_sound_board->current_period < 0.5 / frequency) ? static_cast<double>(INT32_MAX) : static_cast<double>(INT32_MIN));
	}
	simple_sound_board->current_period += sample_rate;
	if (simple_sound_board->current_period >= 1.0 / frequency)
	{
		simple_sound_board->current_period -= 1.0 / frequency;
	}
	return value;
}
