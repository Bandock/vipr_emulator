#ifndef _CDP1863_HPP_
#define _CDP1863_HPP_

#include <cstdint>

namespace VIPR_Emulator
{
	class CDP1863
	{
		public:
			enum class InputClockType
			{
				Clock1, Clock2
			};

			CDP1863(double input_frequency, InputClockType input_clock);
			~CDP1863();

			inline void SetDivideRate(uint8_t value)
			{
				divide_rate = static_cast<double>(value) + 1.0;
			}

			inline void Reset()
			{
				SetDivideRate(53);
			}

			inline double GetOutputFrequency() const
			{
				double fixed_predivide = (input_clock == InputClockType::Clock1) ? 4.0 : 8.0;
				return input_frequency / (fixed_predivide * 2.0) / divide_rate;
			}
		private:
			InputClockType input_clock;
			double input_frequency;
			double divide_rate;
	};
}

#endif
