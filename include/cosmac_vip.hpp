#ifndef _COSMAC_VIP_HPP_
#define _COSMAC_VIP_HPP_

#include "cdp1802.hpp"
#include "cdp1861.hpp"
#include "tone.hpp"
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <fmt/core.h>

namespace VIPR_Emulator
{
	enum class MemoryMapType
	{
		RAM, ROM
	};

	struct MemoryMapData
	{
		uint16_t start_address;
		uint16_t end_address;
		MemoryMapType type;
	};

	class COSMAC_VIP
	{
		public:
			COSMAC_VIP();
			~COSMAC_VIP();
			void SetRunSwitch(bool run);
			
			inline void RunMachine(std::chrono::high_resolution_clock::time_point current_tp)
			{
				CPU(current_tp);
			}

			inline void SetupDisplay(Renderer *DisplayRenderer)
			{
				VDC.AttachDisplayRenderer(DisplayRenderer);
			}

			inline void SetupAudio(std::string output_audio_device)
			{
				tone_generator.SetupToneGenerator(output_audio_device);
			}

			inline bool Fail() const
			{
				return fail;
			}

			inline bool IsRunning() const
			{
				return run;
			}

			inline void SetCPUCycleTimePoint(std::chrono::high_resolution_clock::time_point current_tp)
			{
				CPU.SetCycleTimePoint(current_tp);
			}

			inline void Reset()
			{
				memset(RAM.data(), 0, RAM.size());
				SetRunSwitch(false);
				CPU.Initialize();
			}

			inline void InstallROM(std::vector<uint8_t> &&ROM)
			{
				this->ROM = ROM;
			}

			inline size_t GetRAM() const
			{
				return RAM.size();
			}

			inline uint8_t *GetRAMData()
			{
				return RAM.data();
			}

			inline void AdjustRAM(uint8_t RAM_KB)
			{
				RAM.resize(RAM_KB << 10);
				memset(RAM.data(), 0, RAM.size());
			}

			inline void AdjustVolume(uint8_t volume)
			{
				tone_generator.SetVolume(volume);
			}

			inline void ResetAddressInhibitLatch()
			{
				if (address_inhibit_latch)
				{
					address_inhibit_latch = false;
					MemoryMap[0].type = MemoryMapType::RAM;
				}
			}

			inline void IssueHexKeyPress(uint8_t hex_key)
			{
				hex_key_pressed = true;
				current_hex_key = (hex_key & 0xF);
			}

			inline void IssueHexKeyRelease()
			{
				hex_key_pressed = false;
			}

			friend uint8_t VIP_memory_read(uint16_t address, void *userdata);
			friend void VIP_memory_write(uint16_t address, uint8_t data, void *userdata);
			friend uint8_t VIP_input(uint8_t N, void *userdata);
			friend void VIP_output(uint8_t N, uint8_t data, void *userdata);
			friend void VIP_q_output(uint8_t Q, void *userdata);
			friend void VIP_sync(void *userdata);
		private:
			CDP1802 CPU;
			CDP1861 VDC;
			ToneGenerator tone_generator;
			bool run;
			bool address_inhibit_latch;
			uint8_t hex_key_latch; // 4-bit
			uint8_t current_hex_key; // 4-bit
			bool hex_key_pressed;
			bool *hex_key_press_signal;
			bool fail;
			std::vector<uint8_t> RAM;
			std::vector<uint8_t> ROM;
			std::array<MemoryMapData, 2> MemoryMap =
			{
				MemoryMapData { 0x0000, 0x7FFF, MemoryMapType::RAM }, // RAM
				MemoryMapData { 0x8000, 0xFFFF, MemoryMapType::ROM } // ROM
			};
	};

	uint8_t VIP_memory_read(uint16_t address, void *userdata);
	void VIP_memory_write(uint16_t address, uint8_t data, void *userdata);
	uint8_t VIP_input(uint8_t N, void *userdata);
	void VIP_output(uint8_t N, uint8_t data, void *userdata);
	void VIP_q_output(uint8_t Q, void *userdata);
	void VIP_sync(void *userdata);
}

#endif
