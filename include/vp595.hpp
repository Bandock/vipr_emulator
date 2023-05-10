#ifndef _VP595_HPP_
#define _VP595_HPP_

#include "cdp1863.hpp"
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
			VP595(double input_frequency);
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
				frequency_generator.SetDivideRate(value);
			}

			inline void GenerateTone(bool toggle)
			{
				generate_tone = toggle;
			}

			inline void Reset()
			{
				frequency_generator.Reset();
			}

			inline void Pause(bool toggle)
			{
				pause = toggle;
			}

			static void AudioProcessor(VP595 *generator);
		private:
			SDL_AudioSpec spec;
			SDL_AudioDeviceID device;
			bool processing;
			bool pause;
			bool generate_tone;
			double volume;
			double current_period;
			CDP1863 frequency_generator;
			std::thread AudioProcessingThread;
	};
}

#endif
