#include "cdp1863.hpp"

VIPR_Emulator::CDP1863::CDP1863(double input_frequency, InputClockType input_clock) : input_clock(input_clock), input_frequency(input_frequency), divide_rate(54.0)
{
}

VIPR_Emulator::CDP1863::~CDP1863()
{
}
