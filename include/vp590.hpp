#ifndef _VP590_HPP_
#define _VP590_HPP_

#include <cstdint>
#include <array>

namespace VIPR_Emulator
{
	class VP590
	{
		public:
			VP590();
			~VP590();
		private:
			std::array<uint8_t, 128> color_data_RAM;
	};
}

#endif
