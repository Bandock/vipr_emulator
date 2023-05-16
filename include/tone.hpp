#ifndef _TONE_HPP_
#define _TONE_HPP_

#include <cstdint>
#include <array>
#include <string>
// #include <thread>
// #include <SDL.h>

namespace VIPR_Emulator
{
	class ToneGenerator
	{
		public:
			ToneGenerator();
			~ToneGenerator();

			/*
			void SetupToneGenerator(std::string output_audio_device);

			inline void SetVolume(uint8_t volume)
			{
				this->volume = static_cast<double>(volume) / 100.0;
			}
			*/
			
			inline void GenerateTone(bool toggle)
			{
				generate_tone = toggle;
			}
			
			inline void Pause(bool toggle)
			{
				pause = toggle;
			}

			// static void AudioProcessor(ToneGenerator *generator);

			friend double tone_generator_audio_output_callback(void *source);
		private:
			/*
			SDL_AudioSpec spec;
			SDL_AudioDeviceID device;
			bool processing;
			*/
			bool pause;
			bool generate_tone;
			double volume;
			double current_period;
			// std::thread AudioProcessingThread;
	};

	double tone_generator_audio_output_callback(void *source);
}

#endif
