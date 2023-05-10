#ifndef _CDP1863_HPP_
#define _CDP1863_HPP_

#include <cstdint>

namespace VIPR_Emulator
{
	class CDP1863
	{
		public:
			CDP1863(double input_frequency);
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
				return input_frequency / (divide_rate * 16.0);
			}
		private:
			double input_frequency;
			double divide_rate;
	};
}

#endif
