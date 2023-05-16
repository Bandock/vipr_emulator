#include "audio.hpp"
#include <chrono>
#include <fmt/core.h>

VIPR_Emulator::AudioMixer::AudioMixer() : processing(false), volume(0.5), audio_output(2)
{
}

VIPR_Emulator::AudioMixer::~AudioMixer()
{
	if (processing)
	{
		processing = false;
		AudioProcessingThread.join();
	}
	SDL_PauseAudioDevice(device, 1);
	SDL_CloseAudioDevice(device);
}

void VIPR_Emulator::AudioMixer::SetupAudioMixer(std::string output_audio_device)
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
	desired.channels = 2;
	desired.samples = 4096;
	desired.format = AUDIO_S32;
	device = SDL_OpenAudioDevice(output_audio_device.c_str(), 0, &desired, &spec, 0);
	SDL_PauseAudioDevice(device, 0);
	AudioProcessingThread = std::thread(AudioMixer::AudioProcessor, this);
}

void VIPR_Emulator::AudioMixer::AudioProcessor(AudioMixer *mixer)
{
	std::chrono::high_resolution_clock::time_point audio_tp = std::chrono::high_resolution_clock::now();
	double audio_accumulator = 0.0;
	while (mixer->processing)
	{
		StereoAudioFrame current_frame;
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
			while (audio_accumulator >= 1.0 / static_cast<double>(mixer->spec.freq) && i < current_frame.size())
			{
				if (idle)
				{
					idle = false;
				}
				audio_accumulator -= 1.0 / static_cast<double>(mixer->spec.freq);
				std::array<double, 2> value = { 0.0, 0.0 };
				for (size_t i2 = 0; i2 < mixer->audio_output.size(); ++i2)
				{
					std::lock_guard<std::mutex> AudioOutputLock(mixer->AudioMixerOutputMutex);
					if (mixer->audio_output[i2].callback != nullptr && mixer->audio_output[i2].source != nullptr)
					{
						double output_value = mixer->audio_output[i2].callback(mixer->audio_output[i2].source);
						if (mixer->audio_output[i2].channel & 0x01)
						{
							value[0] += mixer->volume * output_value;
						}
						if (mixer->audio_output[i2].channel & 0x02)
						{
							value[1] += mixer->volume * output_value;
						}
					}
				}
				current_frame[i] = { static_cast<int>(value[0]), static_cast<int>(value[1]) };
				++i;
			}
			if (idle)
			{
				SDL_Delay(1);
			}
		}
		constexpr size_t buffer_max = current_frame.size() * sizeof(StereoAudioData) * 2;
		while (SDL_GetQueuedAudioSize(mixer->device) >= buffer_max)
		{
			SDL_Delay(10);
			audio_tp = std::chrono::high_resolution_clock::now();
		}
		SDL_QueueAudio(mixer->device, current_frame.data(), current_frame.size() * sizeof(StereoAudioData));
	}
}
