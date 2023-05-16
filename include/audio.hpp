#ifndef _AUDIO_HPP_
#define _AUDIO_HPP_

#include <cstdint>
#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <SDL.h>

namespace VIPR_Emulator
{
	struct StereoAudioData
	{
		int left;
		int right;
	};

	using AudioFrame = std::array<int, 4096>;
	using StereoAudioFrame = std::array<StereoAudioData, 4096>;
	using AudioOutputCallback = double (*)(void *userdata);

	struct AudioOutputData
	{
		AudioOutputCallback callback;
		void *source;
		uint8_t channel;
	};

	class AudioMixer
	{
		public:
			AudioMixer();
			~AudioMixer();

			void SetupAudioMixer(std::string output_audio_device);

			inline void SetVolume(uint8_t volume)
			{
				this->volume = static_cast<double>(volume) / 100.0;
			}

			inline void SetAudioOutput(size_t index, AudioOutputCallback callback, void *source, uint8_t channel)
			{
				std::lock_guard<std::mutex> AudioOutputLock(AudioMixerOutputMutex);
				if (index < audio_output.size())
				{
					audio_output[index] = AudioOutputData { callback, source, channel };
				}
			}

			static void AudioProcessor(AudioMixer *mixer);
		private:
			SDL_AudioSpec spec;
			SDL_AudioDeviceID device;
			bool processing;
			double volume;
			std::thread AudioProcessingThread;
			std::vector<AudioOutputData> audio_output;
			std::mutex AudioMixerOutputMutex;
	};
}

#endif
