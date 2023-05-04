#ifndef _VP595_HPP_
#define _VP595_HPP_

#include <cstdint>
#include <array>
#include <string>
#include <thread>
#include <SDL.h>

namespace VIPR_Emulator
{
	class VP595 // VP-595 Simple Sound Board
	{
		public:
			VP595();
			~VP595();

			void SetupVP595(std::string output_audio_device);

			inline void SetVolume(uint8_t volume)
			{
				this->volume = static_cast<double>(volume) / 100.0;
			}

			inline void SetFrequency(uint8_t value)
			{
				if (value == 0x00)
				{
					value = 0x80;
				}
				frequency = input_frequency / ((static_cast<double>(value) + 1) * 16.0);
			}

			inline void GenerateTone(bool toggle)
			{
				generate_tone = toggle;
			}

			static void AudioProcessor(VP595 *generator);
		private:
			SDL_AudioSpec spec;
			SDL_AudioDeviceID device;
			bool processing;
			bool generate_tone;
			const double input_frequency = 440560.0; // Can be modified in a similar fashion to the real board.
			double volume;
			double frequency;
			double current_period;
			std::thread AudioProcessingThread;
	};
}

#endif
