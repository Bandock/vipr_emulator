#ifndef _COSMAC_VIP_HPP_
#define _COSMAC_VIP_HPP_

#include "audio.hpp"
#include "cdp1802.hpp"
#include "cdp1861.hpp"
#include "tone.hpp"
#include "vp590.hpp"
#include "vp595.hpp"
#include "renderer.hpp"
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
		VP585_ExpansionKeypadInterface = 0, // Enables using two keypads; turns off VP590
		VP590_ColorBoard = 1, // Adds color support and enables using two keypads; turns off VP585
		VP595_SimpleSoundBoard = 2 // Adds support for a variable tone generator; replaces the base tone generator
	};

	struct MemoryMapData
	{
		uint16_t start_address;
		uint16_t end_address;
		uint8_t *memory;
		size_t size;
		uint8_t access;
		void (*custom_memory_write)(uint16_t address, uint8_t data, void *userdata);
		void *custom_memory_write_userdata;
	};

	void VIP_video_output(uint8_t value, uint8_t line, size_t address, void *userdata);

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
						case ExpansionBoardType::VP590_ColorBoard:
						{
							VDC = nullptr;
							color_board = std::make_unique<VP590>(&CPU);
							color_board->AttachDisplayRenderer(DisplayRenderer);
							MemoryMap.push_back(MemoryMapData { 0xC000, 0xDFFF, nullptr, 0x1FFF, 0x02, VP590_memory_write, color_board.get() });
							break;
						}
						case ExpansionBoardType::VP595_SimpleSoundBoard:
						{
							mixer.SetAudioOutput(0, nullptr, nullptr, 0x00);
							tone_generator = nullptr;
							simple_sound_board = std::make_unique<VP595>(CPU.GetCycleFrequency() / 8.0); // Uses the Simple Sound Board's oscillator frequency instead of the CPU's.
							mixer.SetAudioOutput(0, VP595_audio_output_callback, simple_sound_board.get(), 0x03);
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
						case ExpansionBoardType::VP590_ColorBoard:
						{
							MemoryMap.pop_back();
							color_board = nullptr;
							VDC = std::make_unique<CDP1861>(&CPU, 0, VIP_video_output, this);
							VDC->AttachDisplayRenderer(DisplayRenderer);
							break;
						}
						case ExpansionBoardType::VP595_SimpleSoundBoard:
						{
							mixer.SetAudioOutput(0, nullptr, nullptr, 0x03);
							simple_sound_board = nullptr;
							tone_generator = std::make_unique<ToneGenerator>();
							mixer.SetAudioOutput(0, tone_generator_audio_output_callback, tone_generator.get(), 0x03);
							break;
						}
					}
				}
			}

			inline void SetupDisplay(Renderer *DisplayRenderer)
			{
				this->DisplayRenderer = DisplayRenderer;
				VDC->AttachDisplayRenderer(this->DisplayRenderer);
			}

			inline void SetupAudio(std::string output_audio_device)
			{
				mixer.SetupAudioMixer(output_audio_device);
				/*
				if (tone_generator != nullptr)
				{
					tone_generator->SetupToneGenerator(output_audio_device);
				}
				else if (simple_sound_board != nullptr)
				{
					simple_sound_board->SetupVP595(output_audio_device);
				}
				*/
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
				if (color_board != nullptr)
				{
					color_board->ClearColorDataRAM();
				}
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
				mixer.SetVolume(volume);
				/*
				if (tone_generator != nullptr)
				{
					tone_generator->SetVolume(volume);
				}
				else if (simple_sound_board != nullptr)
				{
					simple_sound_board->SetVolume(volume);
				}
				*/
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

			inline void IssueHexKeyPress(uint8_t hex_key, uint8_t keypad)
			{
				if (keypad < hex_key_pressed.size())
				{
					hex_key_pressed[keypad] = true;
					current_hex_key[keypad] = (hex_key & 0xF);
				}
			}

			inline void IssueHexKeyRelease(uint8_t keypad)
			{
				if (keypad < hex_key_pressed.size())
				{
					hex_key_pressed[keypad] = false;
				}
			}

			inline void PauseAudio(bool toggle)
			{
				if (tone_generator != nullptr)
				{
					tone_generator->Pause(toggle);
				}
				else if (simple_sound_board != nullptr)
				{
					simple_sound_board->Pause(toggle);
				}
			}

			friend uint8_t VIP_memory_read(uint16_t address, void *userdata);
			friend void VIP_memory_write(uint16_t address, uint8_t data, void *userdata);
			friend uint8_t VIP_input(uint8_t N, void *userdata);
			friend void VIP_output(uint8_t N, uint8_t data, void *userdata);
			friend void VIP_q_output(uint8_t Q, void *userdata);
			friend void VIP_sync(void *userdata);
			friend void VIP_video_output(uint8_t value, uint8_t line, size_t address, void *userdata);
		private:
			CDP1802 CPU;
			std::unique_ptr<CDP1861> VDC;
			AudioMixer mixer;
			std::unique_ptr<ToneGenerator> tone_generator;
			std::unique_ptr<VP590> color_board;
			std::unique_ptr<VP595> simple_sound_board;
			bool run;
			bool address_inhibit_latch;
			uint8_t hex_key_latch; // 4-bit
			std::array<uint8_t, 2> current_hex_key; // 4-bit
			std::array<bool, 2> hex_key_pressed;
			std::array<bool *, 2> hex_key_press_signal;
			bool fail;
			std::vector<uint8_t> RAM;
			std::vector<uint8_t> ROM;
			std::vector<MemoryMapData> MemoryMap;
			std::array<bool, 3> ExpansionBoard;
			Renderer *DisplayRenderer;
	};

	uint8_t VIP_memory_read(uint16_t address, void *userdata);
	void VIP_memory_write(uint16_t address, uint8_t data, void *userdata);
	uint8_t VIP_input(uint8_t N, void *userdata);
	void VIP_output(uint8_t N, uint8_t data, void *userdata);
	void VIP_q_output(uint8_t Q, void *userdata);
	void VIP_sync(void *userdata);
}

#endif
