#include "cdp1862.hpp"

VIPR_Emulator::CDP1862::CDP1862(uint8_t *color_data) : background_color(0x0), color_latch(false), color_data(color_data)
{
}

VIPR_Emulator::CDP1862::~CDP1862()
{
}
