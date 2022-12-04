#include "cosmac_vip.hpp"
#include <chrono>
#include <cstring>
#include <fstream>
#include <fmt/core.h>

VIPR_Emulator::COSMAC_VIP::COSMAC_VIP() : CPU(1760900.0, VIPR_Emulator::VIP_memory_read, VIPR_Emulator::VIP_memory_write, VIPR_Emulator::VIP_input, VIPR_Emulator::VIP_output, VIPR_Emulator::VIP_q_output, VIPR_Emulator::VIP_sync, this), VDC(&CPU, 0), run(false), address_inhibit_latch(true), hex_key_latch(0x0), current_hex_key(0x0), hex_key_pressed(false), hex_key_press_signal(CPU.GetEFPtr(2)), fail(false), RAM(2 << 10)
{
	memset(RAM.data(), 0, RAM.size());
	MemoryMap[0].type = MemoryMapType::ROM;
}

VIPR_Emulator::COSMAC_VIP::~COSMAC_VIP()
{
}

void VIPR_Emulator::COSMAC_VIP::SetRunSwitch(bool run)
{
	if (this->run != run)
	{
		this->run = run;
		if (run)
		{
			CPU.SetControlMode(CDP1802::ControlMode::Run, std::chrono::high_resolution_clock::now());
		}
		else
		{
			VDC.SetDisplay(false);
			VDC.ResetCounters();
			CPU.SetControlMode(CDP1802::ControlMode::Reset, std::chrono::high_resolution_clock::now());
			address_inhibit_latch = true;
			hex_key_latch = 0x0;
			tone_generator.GenerateTone(false);
			MemoryMap[0].type = MemoryMapType::ROM;
		}
	}
}

uint8_t VIPR_Emulator::VIP_memory_read(uint16_t address, void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	for (size_t i = 0; i < VIP->MemoryMap.size(); ++i)
	{
		MemoryMapData &CurrentMemoryMap = VIP->MemoryMap[i];
		if (address >= CurrentMemoryMap.start_address && address <= CurrentMemoryMap.end_address)
		{
			size_t offset = address - CurrentMemoryMap.start_address;
			switch (CurrentMemoryMap.type)
			{
				case MemoryMapType::RAM:
				{
					return (offset < VIP->RAM.size()) ? VIP->RAM[offset] : 0;
				}
				case MemoryMapType::ROM:
				{
					return (offset < VIP->ROM.size()) ? VIP->ROM[offset] : 0;
				}
			}
		}
	}
	return 0;
}

void VIPR_Emulator::VIP_memory_write(uint16_t address, uint8_t data, void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	MemoryMapData &CurrentMemoryMap = VIP->MemoryMap[0];
	if (address >= CurrentMemoryMap.start_address && address <= CurrentMemoryMap.end_address)
	{
		size_t offset = address - CurrentMemoryMap.start_address;
		if (CurrentMemoryMap.type == MemoryMapType::RAM)
		{
			if (offset < VIP->RAM.size())
			{
				VIP->RAM[offset] = data;
			}
		}
	}
}

uint8_t VIPR_Emulator::VIP_input(uint8_t N, void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	switch (N & 0x7)
	{
		case 0x1:
		{
			VIP->VDC.SetDisplay(true);
			return 0;
		}
	}
	return 0;
}

void VIPR_Emulator::VIP_output(uint8_t N, uint8_t data, void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	switch (N & 0x7)
	{
		case 0x1:
		{
			VIP->VDC.SetDisplay(false);
			break;
		}
		case 0x2:
		{
			VIP->hex_key_latch = (data & 0xF);
			break;
		}
		case 0x3:
		{
			break;
		}
		case 0x4:
		{
			VIP->ResetAddressInhibitLatch();
			break;
		}
	}
}

void VIPR_Emulator::VIP_q_output(uint8_t Q, void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	if (Q)
	{
		VIP->tone_generator.GenerateTone(true);
	}
	else
	{
		VIP->tone_generator.GenerateTone(false);
	}
}

void VIPR_Emulator::VIP_sync(void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	*VIP->hex_key_press_signal = (VIP->current_hex_key == VIP->hex_key_latch && VIP->hex_key_pressed);
	VIP->VDC.Sync();
}
