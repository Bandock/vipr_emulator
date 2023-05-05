#include "cosmac_vip.hpp"
#include <chrono>
#include <cstring>
#include <fstream>
#include <fmt/core.h>

VIPR_Emulator::COSMAC_VIP::COSMAC_VIP() : CPU(1760900.0, VIPR_Emulator::VIP_memory_read, VIPR_Emulator::VIP_memory_write, VIPR_Emulator::VIP_input, VIPR_Emulator::VIP_output, VIPR_Emulator::VIP_q_output, VIPR_Emulator::VIP_sync, this), VDC(&CPU, 0), tone_generator(nullptr), simple_sound_board(nullptr), run(false), address_inhibit_latch(true), hex_key_latch(0x0), current_hex_key { 0x0, 0x0 }, hex_key_pressed { false, false }, hex_key_press_signal { CPU.GetEFPtr(2), CPU.GetEFPtr(3) }, fail(false), RAM(2 << 10)
{
	memset(RAM.data(), 0, RAM.size());
	MemoryMap.resize(2);
	MemoryMap[0] = MemoryMapData { 0x0000, 0x7FFF, ROM.data(), ROM.size(), 0x01 };
	MemoryMap[1] = MemoryMapData { 0x8000, 0xFFFF, ROM.data(), ROM.size(), 0x01 };
	tone_generator = std::make_unique<ToneGenerator>();
	for (size_t i = 0; i < ExpansionBoard.size(); ++i)
	{
		ExpansionBoard[i] = false;
	}
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
			if (tone_generator != nullptr)
			{
				tone_generator->GenerateTone(false);
			}
			else if (simple_sound_board != nullptr)
			{
				simple_sound_board->GenerateTone(false);
				simple_sound_board->SetFrequency(0x00);
			}
			MemoryMap[0].memory = ROM.data();
			MemoryMap[0].size = ROM.size();
			MemoryMap[0].access = 0x01;
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
			return ((CurrentMemoryMap.access & 0x01) && offset < CurrentMemoryMap.size) ? CurrentMemoryMap.memory[offset] : 0;
			/*
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
			*/
		}
	}
	return 0;
}

void VIPR_Emulator::VIP_memory_write(uint16_t address, uint8_t data, void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	for (size_t i = 0; i < VIP->MemoryMap.size(); ++i)
	{
		MemoryMapData &CurrentMemoryMap = VIP->MemoryMap[i];
		if (address >= CurrentMemoryMap.start_address && address <= CurrentMemoryMap.end_address)
		{
			size_t offset = address - CurrentMemoryMap.start_address;
			if (CurrentMemoryMap.access & 0x02)
			{
				if (offset < CurrentMemoryMap.size)
				{
					CurrentMemoryMap.memory[offset] = data;
				}
			}
		}
		/*
		if (CurrentMemoryMap.type == MemoryMapType::RAM)
		{
			if (offset < VIP->RAM.size())
			{
				VIP->RAM[offset] = data;
			}
		}
		*/
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
			if (VIP->ExpansionBoard[static_cast<uint8_t>(ExpansionBoardType::VP595_SimpleSoundBoard)])
			{
				VIP->simple_sound_board->SetFrequency(data);
			}
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
		if (VIP->tone_generator != nullptr)
		{
			VIP->tone_generator->GenerateTone(true);
		}
		else if (VIP->simple_sound_board != nullptr)
		{
			VIP->simple_sound_board->GenerateTone(true);
		}
	}
	else
	{
		if (VIP->tone_generator != nullptr)
		{
			VIP->tone_generator->GenerateTone(false);
		}
		else if (VIP->simple_sound_board != nullptr)
		{
			VIP->simple_sound_board->GenerateTone(false);
		}
	}
}

void VIPR_Emulator::VIP_sync(void *userdata)
{
	COSMAC_VIP *VIP = static_cast<COSMAC_VIP *>(userdata);
	for (size_t i = 0; i < VIP->hex_key_press_signal.size(); ++i)
	{
		if (i == 1 && (!VIP->ExpansionBoard[static_cast<uint8_t>(ExpansionBoardType::VP585_ExpansionKeypadInterface)] && !VIP->ExpansionBoard[static_cast<uint8_t>(ExpansionBoardType::VP590_ColorBoard)]))
		{
			break;
		}
		*VIP->hex_key_press_signal[i] = (VIP->current_hex_key[i] == VIP->hex_key_latch && VIP->hex_key_pressed[i]);
	}
	VIP->VDC.Sync();
}
