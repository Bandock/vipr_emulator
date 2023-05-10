#include "vp590.hpp"
#include <cstring>

VIPR_Emulator::VP590::VP590(CDP1802 *CPU) : VDC(CPU, 0, VP590_video_output, this), color_generator(color_data_RAM.data()), current_resolution_mode(ResolutionMode::Low), DisplayRenderer(nullptr)
{
	memset(color_data_RAM.data(), 0x00, color_data_RAM.size());
}

VIPR_Emulator::VP590::~VP590()
{
}

void VIPR_Emulator::VP590::Sync()
{
	VDC.Sync();
}

void VIPR_Emulator::VP590_video_output(uint8_t value, uint8_t line, size_t address, void *userdata)
{
	VP590 *ColorBoard = static_cast<VP590 *>(userdata);
	uint8_t x = address;
	uint8_t y = (ColorBoard->GetResolutionMode() == VP590::ResolutionMode::Low) ? (line / 32) * 8: (line / 4);
	ColorBoard->DisplayRenderer->DrawByte(value, line, address, ColorBoard->color_generator.GetBackgroundColor(), ColorBoard->color_generator.GetDotColor(x, y));
}

void VIPR_Emulator::VP590_memory_write(uint16_t address, uint8_t data, void *userdata)
{
	VP590 *ColorBoard = static_cast<VP590 *>(userdata);
	ColorBoard->color_generator.ColorLatchEnable();
	ColorBoard->current_resolution_mode = !(address & 0x1000) ? VP590::ResolutionMode::Low : VP590::ResolutionMode::High;
	uint16_t data_address = (address & 0xFFF);
	switch (ColorBoard->current_resolution_mode)
	{
		case VP590::ResolutionMode::Low:
		{
			for (uint16_t offset = 0x00; offset < 0x100; offset += 0x20)
			{
				if (data_address >= offset && data_address <= (offset + 0x07))
				{
					size_t color_data_RAM_address = (data_address / 2);
					size_t color_data_RAM_bit_offset = (data_address % 2) * 4;
					ColorBoard->color_data_RAM[color_data_RAM_address] &= ~(0xF << color_data_RAM_bit_offset);
					ColorBoard->color_data_RAM[color_data_RAM_address] |= ((data & 0xF) << color_data_RAM_bit_offset);
					break;
				}
			}
			break;
		}
		case VP590::ResolutionMode::High:
		{
			if (data_address < 0x100)
			{
				size_t color_data_RAM_address = (data_address / 2);
				size_t color_data_RAM_bit_offset = (data_address % 2) * 4;
				ColorBoard->color_data_RAM[color_data_RAM_address] &= ~(0xF << color_data_RAM_bit_offset);
				ColorBoard->color_data_RAM[color_data_RAM_address] |= ((data & 0xF) << color_data_RAM_bit_offset);
			}
			break;
		}
	}
}
