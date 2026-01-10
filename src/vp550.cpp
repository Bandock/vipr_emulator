#include "vp550.hpp"

VIPR_Emulator::VP550::VP550(double input_frequency) : generate_tone(false)
{
	for (size_t i = 0; i < frequency_generator.size(); ++i)
	{
		frequency_generator[i] = CDP1863(input_frequency, CDP1863::InputClockType::Clock1);
	}
}

VIPR_Emulator::VP550::~VP550()
{

}