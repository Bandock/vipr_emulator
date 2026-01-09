#include "audio.hpp"
#include <chrono>
#include <fmt/core.h>

VIPR_Emulator::AudioMixer::AudioMixer() : playback_stream(nullptr), processing(false), volume(0.5), audio_output(2)
{
	SDL_zero(spec);
}

VIPR_Emulator::AudioMixer::~AudioMixer()
{
	if (processing)
	{
		processing = false;
		AudioProcessingThread.join();
	}
	if (playback_stream != nullptr)
	{
		SDL_UnbindAudioStream(playback_stream);
	}
	SDL_CloseAudioDevice(device);
}

void VIPR_Emulator::AudioMixer::SetupAudioMixer(std::string output_audio_device)
{
	if (processing)
	{
		processing = false;
		AudioProcessingThread.join();
		if (playback_stream != nullptr)
		{
			SDL_UnbindAudioStream(playback_stream);
			SDL_DestroyAudioStream(playback_stream);
		}
		SDL_CloseAudioDevice(device);
	}
	processing = true;
	int device_count = 0;
	SDL_AudioDeviceID* playback_devices = SDL_GetAudioPlaybackDevices(&device_count);
	for (int i = 0; i < device_count; ++i)
	{
		std::string device_name = SDL_GetAudioDeviceName(playback_devices[i]);
		if (device_name == output_audio_device)
		{
			device = SDL_OpenAudioDevice(playback_devices[i], nullptr);
			spec.freq = 192000;
			spec.channels = 2;
			spec.format = SDL_AUDIO_S32LE;
			playback_stream = SDL_CreateAudioStream(&spec, nullptr);
			SDL_BindAudioStreams(device, &playback_stream, 1);
			break;
		}
	}
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
		if (mixer->playback_stream != nullptr)
		{
			while (SDL_GetAudioStreamQueued(mixer->playback_stream) >= buffer_max)
			{
				SDL_Delay(10);
				audio_tp = std::chrono::high_resolution_clock::now();
				if (!mixer->processing)
				{
					SDL_ClearAudioStream(mixer->playback_stream);
				}
			}
			if (mixer->processing)
			{
				SDL_PutAudioStreamData(mixer->playback_stream, current_frame.data(), current_frame.size() * sizeof(StereoAudioData));
			}
		}
	}
}
