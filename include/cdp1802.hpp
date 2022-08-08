#ifndef _CDP1802_HPP_
#define _CDP1802_HPP_

#include <cstdint>
#include <chrono>
#include <array>
#include <deque>

namespace VIPR_Emulator
{
	using MemoryReadCallback = uint8_t (*)(uint16_t address, void *userdata);
	using MemoryWriteCallback = void (*)(uint16_t address, uint8_t data, void *userdata);
	
	using InputCallback = uint8_t (*)(uint8_t N, void *userdata);
	using OutputCallback = void (*)(uint8_t N, uint8_t data, void *userdata);
	
	using DMACallback = void (*)(uint8_t *data, void *userdata);

	using QOutputCallback = void (*)(uint8_t Q, void *userdata);
	using SyncCallback = void (*)(void *userdata);

	enum class DMAType
	{
		None, DMAIn, DMAOut
	};

	struct DMARequest
	{
		uint16_t bytes_to_transfer;
		void *userdata;
		DMACallback func;
	};

	struct DMATransferData
	{
		DMAType type;
		void *userdata;
		DMACallback func;
	};

	class CDP1802
	{
		public:
			enum class ControlMode
			{
				Load,
				Reset,
				Pause,
				Run
			};
			enum class CycleState
			{
				Fetch,
				Execute,
				DMA,
				Interrupt
			};
			CDP1802(double cycle_frequency, MemoryReadCallback memory_read_func, MemoryWriteCallback memory_write_func, InputCallback in_func, OutputCallback out_func, QOutputCallback qout_func, SyncCallback sync_func, void *userdata);
			~CDP1802();
			void Initialize();

			inline void SetCycleTimePoint(std::chrono::high_resolution_clock::time_point current_tp)
			{
				cycle_tp = current_tp;
			}

			void SetControlMode(ControlMode mode, std::chrono::high_resolution_clock::time_point current_tp);
			ControlMode GetControlMode() const;

			inline bool *GetEFPtr(uint8_t index)
			{
				return (index < EF.size()) ? &EF[index] : nullptr;
			}

			inline void IssueDMAInRequest(uint16_t bytes_to_transfer, void *userdata, DMACallback dma_in_func)
			{
				if (!dma_in_request)
				{
					dma_in_request = true;
					CurrentDMAInRequest = { bytes_to_transfer, userdata, dma_in_func };
				}
			}

			inline void IssueDMAOutRequest(uint16_t bytes_to_transfer, void *userdata, DMACallback dma_out_func)
			{
				if (!dma_out_request)
				{
					dma_out_request = true;
					CurrentDMAOutRequest = { bytes_to_transfer, userdata, dma_out_func };
				}
			}

			inline void IssueInterruptRequest()
			{
				if (!interrupt_request)
				{
					interrupt_request = true;
				}
			}
			
			void operator()(std::chrono::high_resolution_clock::time_point current_tp);
		private:
			ControlMode CurrentControlMode;
			CycleState CurrentCycleState;
			DMARequest CurrentDMAInRequest, CurrentDMAOutRequest;
			double cycle_frequency;
			uint32_t current_clock;
			uint32_t execute_cycles_left;
			bool initialization;
			bool idle;
			bool dma_in_request;
			bool dma_out_request;
			bool interrupt_request;
			uint8_t D; // 8-bit Data Register (Accumulator)
			uint8_t DF; // 1-bit Data Flag (ALU Carry)
			uint8_t B; // 8-bit Auxiliary Holding Register
			std::array<uint16_t, 16> R; // 16-bit Scratchpad Registers
			uint8_t P; // 4-bit Program Counter Register
			uint8_t X; // 4-bit Data Pointer Register
			uint8_t N; // 4-bit Low-Order Instruction Digit
			uint8_t I; // 4-bit High-Order Instruction Digit
			uint8_t T; // 8-bit, Holds P, X after Interrupt (X = High Nibble)
			uint8_t IE; // 1-bit Interrupt Enable
			uint8_t Q; // Output Flip-Flop
			bool N0; // IO Control Line 0
			bool N1; // IO Control Line 1
			bool N2; // IO Control Line 2
			std::array<bool, 4> EF; // Flags
			std::chrono::high_resolution_clock::time_point cycle_tp;
			double cycle_accumulator;
			std::deque<DMATransferData> DMATransferQueue;
			void *userdata;
			MemoryReadCallback memory_read_func;
			MemoryWriteCallback memory_write_func;
			InputCallback in_func;
			OutputCallback out_func;
			QOutputCallback qout_func;
			SyncCallback sync_func;
	};
}

#endif
