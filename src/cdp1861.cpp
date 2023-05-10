#include "cdp1861.hpp"

VIPR_Emulator::CDP1861::CDP1861(CDP1802 *CPU, uint8_t EFX, VideoOutputCallback video_output_func, void *video_output_userdata) : CPU(CPU), EFX(nullptr), SC0(false), SC1(false), display(false), line_counter(0), display_memory_address(0), machine_cycle_counter(0), video_output_func(video_output_func), video_output_userdata(video_output_userdata), DisplayRenderer(nullptr)
{
	if (this->CPU != nullptr)
	{
		this->EFX = this->CPU->GetEFPtr(EFX);
	}
}

VIPR_Emulator::CDP1861::~CDP1861()
{
}

void VIPR_Emulator::CDP1861_DMA_out(uint8_t *data, void *userdata)
{
	CDP1861 *VDC = static_cast<CDP1861 *>(userdata);
	constexpr uint8_t bytes_per_line = 64 / 8;
	/*
	if (VDC->DisplayRenderer != nullptr)
	{
		VDC->DisplayRenderer->DrawByte(*data, VDC->line_counter - 64, VDC->display_memory_address % bytes_per_line, 1, 7);
	}
	*/
	if (VDC->video_output_func != nullptr)
	{
		VDC->video_output_func(*data, VDC->line_counter - 64, VDC->display_memory_address % bytes_per_line, VDC->video_output_userdata);
	}
	++VDC->display_memory_address;
	constexpr uint16_t max_display_size = (64 * 128) / 8;
	if (VDC->display_memory_address == max_display_size)
	{
		VDC->display_memory_address = 0;
	}
}
