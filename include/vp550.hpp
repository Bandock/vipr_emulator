#ifndef _VP550_HPP_
#define _VP550_HPP_

#include "cdp1863.hpp"
#include <cstdint>
#include <array>
#include <string>

namespace VIPR_Emulator
{
	class VP550 // VP-550 Super Sound System
	{
		public:
			VP550(double input_frequency);
			~VP550();

			inline void SetFrequency(uint8_t channel, uint8_t value)
			{
				if (value == 0x00)
				{
					value = 0x80;
				}
				frequency_generator[channel].SetDivideRate(value);
			}

			inline void GenerateTone(uint8_t channel)
			{

			}
		private:
			bool generate_tone;
			std::array<double, 2> volume;
			std::array<double, 2> current_period;
			std::array<CDP1863, 2> frequency_generator;
	};
}

#endif
