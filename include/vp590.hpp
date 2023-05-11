#ifndef _VP590_HPP_
#define _VP590_HPP_

#include "cdp1802.hpp"
#include "cdp1861.hpp"
#include "cdp1862.hpp"
#include "renderer.hpp"
#include <cstdint>
#include <array>
#include <cstring>

namespace VIPR_Emulator
{
	class VP590 // VP-590 Color Board
	{
		public:
			enum class ResolutionMode
			{
				Low, High
			};

			VP590(CDP1802 *CPU);
			~VP590();

			inline void AttachDisplayRenderer(Renderer *DisplayRenderer)
			{
				this->DisplayRenderer = DisplayRenderer;
				VDC.AttachDisplayRenderer(DisplayRenderer);
			}

			inline void SetDisplay(bool toggle)
			{
				VDC.SetDisplay(toggle);
			}

			inline bool GetDisplay() const
			{
				return VDC.GetDisplay();
			}

			inline void ResetCounters()
			{
				VDC.ResetCounters();
			}

			inline void ResetColorGenerator()
			{
				color_generator.Reset();
			}

			inline void ClearColorDataRAM()
			{
				memset(color_data_RAM.data(), 0, color_data_RAM.size());
			}

			inline void StepBackgroundColor()
			{
				color_generator.StepBackgroundColor();
			}

			inline ResolutionMode GetResolutionMode() const
			{
				return current_resolution_mode;
			}

			void Sync();

			friend void VP590_video_output(uint8_t value, uint8_t line, size_t address, void *userdata);
			friend void VP590_memory_write(uint16_t address, uint8_t data, void *userdata);
		private:
			CDP1861 VDC;
			CDP1862 color_generator;
			ResolutionMode current_resolution_mode;
			std::array<uint8_t, 128> color_data_RAM;
			Renderer *DisplayRenderer;
	};

	void VP590_video_output(uint8_t value, uint8_t line, size_t address, void *userdata);
	void VP590_memory_write(uint16_t address, uint8_t data, void *userdata);
}

#endif
