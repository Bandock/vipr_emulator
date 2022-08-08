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
	void CDP1861_DMA_out(uint8_t *data, void *userdata);

	class CDP1861
	{
		public:
			CDP1861(CDP1802 *CPU, uint8_t EFX);
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
					if (line_counter >= 64 && line_counter <= 191)
					{
						if (machine_cycle_counter == 2)
						{
							CPU->IssueDMAOutRequest(8, this, CDP1861_DMA_out);
						}
					}
					if (line_counter == 62 && machine_cycle_counter == 0)
					{
						CPU->IssueInterruptRequest();
					}
					if (EFX != nullptr && machine_cycle_counter == 0)
					{
						if ((line_counter >= 60 && line_counter <= 63) || (line_counter >= 188 && line_counter <= 191))
						{
							*EFX = true;
						}
						else
						{
							*EFX = false;
						}
					}
				}
				if (line_counter == 192 && machine_cycle_counter == 0 && DisplayRenderer != nullptr)
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
			Renderer *DisplayRenderer;
	};
}

#endif
