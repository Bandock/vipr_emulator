#ifndef _COSMAC_VIP_HPP_
#define _COSMAC_VIP_HPP_

#include "cdp1802.hpp"
#include "cdp1861.hpp"
#include "tone.hpp"
#include "vp595.hpp"
#include <cstdint>
#include <memory>
#include <array>
#include <vector>
#include <string>
#include <chrono>
#include <fmt/core.h>

namespace VIPR_Emulator
{
	/*
	enum class MemoryMapType
	{
		RAM, ROM
	};
	*/

	enum class ExpansionBoardType : uint8_t
	{
		VP590_ColorBoard = 0,
		VP595_SimpleSoundBoard = 1
	};

	struct MemoryMapData
	{
		uint16_t start_address;
		uint16_t end_address;
		uint8_t *memory;
		size_t size;
		uint8_t access;
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

			inline void InstallExpansionBoard(ExpansionBoardType board)
			{
				uint8_t current_expansion_board_type = static_cast<uint8_t>(board);
				if (!ExpansionBoard[current_expansion_board_type])
				{
					ExpansionBoard[current_expansion_board_type] = true;
					switch (board)
					{
						case ExpansionBoardType::VP595_SimpleSoundBoard:
						{
							tone_generator = nullptr;
							simple_sound_board = std::make_unique<VP595>();
							break;
						}
					}
				}
			}

			inline void UninstallExpansionBoard(ExpansionBoardType board)
			{
				uint8_t current_expansion_board_type = static_cast<uint8_t>(board);
				if (ExpansionBoard[current_expansion_board_type])
				{
					ExpansionBoard[current_expansion_board_type] = false;
					switch (board)
					{
						case ExpansionBoardType::VP595_SimpleSoundBoard:
						{
							simple_sound_board = nullptr;
							tone_generator = std::make_unique<ToneGenerator>();
							break;
						}
					}
				}
			}

			inline void SetupDisplay(Renderer *DisplayRenderer)
			{
				VDC.AttachDisplayRenderer(DisplayRenderer);
			}

			inline void SetupAudio(std::string output_audio_device)
			{
				if (tone_generator != nullptr)
				{
					tone_generator->SetupToneGenerator(output_audio_device);
				}
				else if (simple_sound_board != nullptr)
				{
					simple_sound_board->SetupVP595(output_audio_device);
				}
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
				MemoryMap[0].memory = this->ROM.data();
				MemoryMap[0].size = this->ROM.size();
				MemoryMap[1].memory = this->ROM.data();
				MemoryMap[1].size = this->ROM.size();
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
				if (tone_generator != nullptr)
				{
					tone_generator->SetVolume(volume);
				}
				else if (simple_sound_board != nullptr)
				{
					simple_sound_board->SetVolume(volume);
				}
			}

			inline void ResetAddressInhibitLatch()
			{
				if (address_inhibit_latch)
				{
					address_inhibit_latch = false;
					MemoryMap[0].memory = RAM.data();
					MemoryMap[0].size = RAM.size();
					MemoryMap[0].access |= 0x02;
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
			std::unique_ptr<ToneGenerator> tone_generator;
			std::unique_ptr<VP595> simple_sound_board;
			bool run;
			bool address_inhibit_latch;
			uint8_t hex_key_latch; // 4-bit
			uint8_t current_hex_key; // 4-bit
			bool hex_key_pressed;
			bool *hex_key_press_signal;
			bool fail;
			std::vector<uint8_t> RAM;
			std::vector<uint8_t> ROM;
			std::vector<MemoryMapData> MemoryMap;
			std::array<bool, 2> ExpansionBoard;
	};

	uint8_t VIP_memory_read(uint16_t address, void *userdata);
	void VIP_memory_write(uint16_t address, uint8_t data, void *userdata);
	uint8_t VIP_input(uint8_t N, void *userdata);
	void VIP_output(uint8_t N, uint8_t data, void *userdata);
	void VIP_q_output(uint8_t Q, void *userdata);
	void VIP_sync(void *userdata);
}

#endif
