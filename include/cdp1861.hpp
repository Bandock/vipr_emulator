#ifndef _CDP1861_HPP_
#define _CDP1861_HPP_

#include "cdp1802.hpp"
#include "renderer.hpp"
#include <cstdint>
#include <array>
#include <chrono>
#include <cstring>
#include <fmt/core.h>

namespace VIPR_Emulator
{
	using VideoOutputCallback = void (*)(uint8_t value, uint8_t line, size_t address, void *userdata);

	void CDP1861_DMA_out(uint8_t *data, void *userdata);

	const uint8_t vertical_sync_line_count = 16;

	class CDP1861
	{
		public:
			CDP1861(CDP1802 *CPU, uint8_t EFX, VideoOutputCallback video_output_func, void *video_output_userdata);
			~CDP1861();

			inline void AttachDisplayRenderer(Renderer *DisplayRenderer)
			{
				this->DisplayRenderer = DisplayRenderer;
			}
			
			inline void SetDisplay(bool toggle)
			{
				if (display != toggle)
				{
					display = toggle;
					if (!display)
					{
						display_memory_address = 0;
						if (DisplayRenderer != nullptr)
						{
							DisplayRenderer->ClearDisplay();
							DisplayRenderer->Render();
						}
					}
				}
			}

			inline bool GetDisplay() const
			{
				return display;
			}

			inline void ResetCounters()
			{
				machine_cycle_counter = 0;
				line_counter = 0;
			}

			inline void Sync()
			{
				if (display)
				{
					if (line_counter >= (64 + vertical_sync_line_count) && line_counter <= (191 + vertical_sync_line_count))
					{
						if (machine_cycle_counter == 2)
						{
							CPU->IssueDMAOutRequest(8, this, CDP1861_DMA_out);
						}
					}
					if (line_counter == (62 + vertical_sync_line_count) && machine_cycle_counter == 0)
					{
						CPU->IssueInterruptRequest();
					}
					if (EFX != nullptr && machine_cycle_counter == 0)
					{
						if ((line_counter >= (60 + vertical_sync_line_count) && line_counter <= (63 + vertical_sync_line_count)) || (line_counter >= (188 + vertical_sync_line_count) && line_counter <= (191 + vertical_sync_line_count)))
						{
							*EFX = true;
						}
						else
						{
							*EFX = false;
						}
					}
				}
				if (line_counter == (192 + vertical_sync_line_count) && machine_cycle_counter == 0 && DisplayRenderer != nullptr)
				{
					DisplayRenderer->Render();
				}
				++machine_cycle_counter;
				if (machine_cycle_counter == 14)
				{
					machine_cycle_counter = 0;
					++line_counter;
					if (line_counter == 262)
					{
						line_counter = 0;
					}
				}
			}

			friend void CDP1861_DMA_out(uint8_t *data, void *userdata);
		private:
			CDP1802 *CPU;
			bool *EFX;
			bool SC0;
			bool SC1;
			bool display;
			uint16_t line_counter;
			uint16_t display_memory_address;
			uint8_t machine_cycle_counter;
			void *video_output_userdata;
			VideoOutputCallback video_output_func;
			Renderer *DisplayRenderer;
	};
}

#endif
