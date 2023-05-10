#ifndef _CDP1862_HPP_
#define _CDP1862_HPP_

#include <cstdint>

namespace VIPR_Emulator
{
	class CDP1862
	{
		public:
			CDP1862(uint8_t *color_data);
			~CDP1862();

			inline void Reset()
			{
				background_color = 0x0;
				color_latch = false;
			}

			inline void ColorLatchEnable()
			{
				if (!color_latch)
				{
					color_latch = true;
				}
			}

			inline void StepBackgroundColor()
			{
				background_color = (background_color + 1) & 0x3;
			}

			uint8_t GetBackgroundColor() const
			{
				return background_color;
			}

			uint8_t GetDotColor(uint8_t x, uint8_t y)
			{
				if (!color_latch)
				{
					return 7;
				}
				else
				{
					std::size_t address = ((y * 8) + x) / 2;
					std::size_t bit_offset = (((y * 8) + x) % 2) * 4;
					uint8_t dot_color = ((color_data[address] & (0x7 << bit_offset)) >> bit_offset);
					return dot_color;
				}
			}
		private:
			uint8_t background_color;
			bool color_latch;
			uint8_t *color_data;
	};
}

#endif
