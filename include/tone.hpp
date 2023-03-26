#ifndef _TONE_HPP_
#define _TONE_HPP_

#include <cstdint>
#include <array>
#include <string>
#include <thread>
#include <SDL.h>

namespace VIPR_Emulator
{
	using AudioFrame = std::array<int, 4096>;

	class ToneGenerator
	{
		public:
			ToneGenerator();
			~ToneGenerator();

			void SetupToneGenerator(std::string output_audio_device);
			
			inline void GenerateTone(bool toggle)
			{
				generate_tone = toggle;
			}
			
			static void AudioProcessor(ToneGenerator *generator);
		private:
			SDL_AudioSpec spec;
			SDL_AudioDeviceID device;
			bool processing;
			bool generate_tone;
			double current_period;
			std::thread AudioProcessingThread;
	};
}

#endif
