#include "cdp1802.hpp"
#include <fmt/core.h>

VIPR_Emulator::CDP1802::CDP1802(double cycle_frequency, MemoryReadCallback memory_read_func, MemoryWriteCallback memory_write_func, InputCallback in_func, OutputCallback out_func, QOutputCallback qout_func, SyncCallback sync_func, void *userdata) : CurrentControlMode(ControlMode::Reset), CurrentCycleState(CycleState::Execute), CurrentDMAInRequest{ 0, nullptr, nullptr }, CurrentDMAOutRequest{ 0, nullptr, nullptr }, current_clock(9), execute_cycles_left(0), initialization(false), idle(false), dma_in_request(false), dma_out_request(false), interrupt_request(false), D(0x00), DF(0), B(0x00), P(0x0), X(0x0), N(0x0), I(0x0), T(0x00), IE(0x1), Q(0), N0(false), N1(false), N2(false), cycle_accumulator(0.0), userdata(userdata), memory_read_func(memory_read_func), memory_write_func(memory_write_func), in_func(in_func), out_func(out_func), qout_func(qout_func), sync_func(sync_func)
{
	if (cycle_frequency > 6400000.0)
	{
		cycle_frequency = 6400000.0;
	}
	this->cycle_frequency = cycle_frequency;
	for (uint8_t i = 0; i < R.size(); ++i)
	{
		R[i] = 0x0000;
	}
	for (uint8_t i = 0; i < EF.size(); ++i)
	{
		EF[i] = false;
	}
}

VIPR_Emulator::CDP1802::~CDP1802()
{
}

void VIPR_Emulator::CDP1802::Initialize()
{
	for (uint8_t i = 0; i < R.size(); ++i)
	{
		R[i] = 0x0000;
	}
	for (uint8_t i = 0; i < EF.size(); ++i)
	{
		EF[i] = false;
	}
	D = 0x00;
	DF = 0;
	B = 0x00;
	T = 0x00;
	N0 = false;
	N1 = false;
	N2 = false;
}

void VIPR_Emulator::CDP1802::SetControlMode(VIPR_Emulator::CDP1802::ControlMode mode, std::chrono::high_resolution_clock::time_point current_tp)
{
	if (CurrentControlMode != mode)
	{
		switch (mode)
		{
			case ControlMode::Load:
			{
				break;
			}
			case ControlMode::Reset:
			{
				I = 0x0;
				N = 0x0;
				Q = 0;
				IE = 0x1;
				CurrentCycleState = CycleState::Execute;
				CurrentDMAInRequest = { 0, nullptr, nullptr };
				CurrentDMAOutRequest = { 0, nullptr, nullptr };
				while (DMATransferQueue.size() > 0)
				{
					DMATransferQueue.pop_front();
				}
				cycle_accumulator = 0.0;
				idle = false;
				dma_in_request = false;
				dma_out_request = false;
				interrupt_request = false;
				break;
			}
			case ControlMode::Pause:
			{
				std::chrono::duration<double> delta_time = current_tp - cycle_tp;
				if (delta_time.count() > 0.25)
				{
					delta_time = std::chrono::duration<double>(0.25);
				}
				cycle_accumulator += delta_time.count();
				break;
			}
			case ControlMode::Run:
			{ 
				cycle_tp = current_tp;
				if (CurrentControlMode == ControlMode::Reset)
				{
					initialization = true;
					current_clock = 9;
				}
				break;
			}
		}
		CurrentControlMode = mode;
	}
}

VIPR_Emulator::CDP1802::ControlMode VIPR_Emulator::CDP1802::GetControlMode() const
{
	return CurrentControlMode;
}

void VIPR_Emulator::CDP1802::operator()(std::chrono::high_resolution_clock::time_point current_tp)
{
	std::chrono::duration<double> delta_time = current_tp - cycle_tp;
	if (delta_time.count() > 0.25)
	{
		delta_time = std::chrono::duration<double>(0.25);
	}
	cycle_accumulator += delta_time.count();
	cycle_tp = current_tp;
	double cycle_rate = 1.0 / cycle_frequency;
	for (; cycle_accumulator >= cycle_rate; cycle_accumulator -= cycle_rate)
	{
		--current_clock;
		if (!current_clock)
		{
			current_clock = 8;
			if (qout_func != nullptr && userdata != nullptr)
			{
				qout_func(Q, userdata);
			}
			if (sync_func != nullptr && userdata != nullptr)
			{
				sync_func(userdata);
			}
			switch (CurrentCycleState)
			{
				case CycleState::Fetch:
				{
					uint8_t data = 0;
					if (memory_read_func != nullptr && userdata != nullptr)
					{
						data = memory_read_func(R[P], userdata);
					}
					I = (data >> 4);
					N = (data & 0xF);
					switch (data)
					{
						case 0x00:
						{
							idle = true;
							break;
						}
						case 0xC0:
						case 0xC1:
						case 0xC2:
						case 0xC3:
						case 0xC4:
						case 0xC5:
						case 0xC6:
						case 0xC7:
						case 0xC8:
						case 0xC9:
						case 0xCA:
						case 0xCB:
						case 0xCC:
						case 0xCD:
						case 0xCE:
						case 0xCF:
						{
							execute_cycles_left = 2;
							break;
						}
						default:
						{
							execute_cycles_left = 1;
							break;
						}
					}
					++R[P];
					CurrentCycleState = CycleState::Execute;
					break;
				}
				case CycleState::Execute:
				{
					if (initialization)
					{
						X = 0x0;
						P = 0x0;
						R[0] = 0x0000;
						CurrentCycleState = CycleState::Fetch;
						initialization = false;
					}
					else if (!idle)
					{
						uint8_t current_instruction = (I << 4) | N;
						switch (current_instruction)
						{
							case 0x01:
							case 0x02:
							case 0x03:
							case 0x04:
							case 0x05:
							case 0x06:
							case 0x07:
							case 0x08:
							case 0x09:
							case 0x0A:
							case 0x0B:
							case 0x0C:
							case 0x0D:
							case 0x0E:
							case 0x0F:
							{
								D = (memory_read_func != nullptr) ? memory_read_func(R[N], userdata) : 0;
								break;
							}
							case 0x10:
							case 0x11:
							case 0x12:
							case 0x13:
							case 0x14:
							case 0x15:
							case 0x16:
							case 0x17:
							case 0x18:
							case 0x19:
							case 0x1A:
							case 0x1B:
							case 0x1C:
							case 0x1D:
							case 0x1E:
							case 0x1F:
							{
								++R[N];
								break;
							}
							case 0x20:
							case 0x21:
							case 0x22:
							case 0x23:
							case 0x24:
							case 0x25:
							case 0x26:
							case 0x27:
							case 0x28:
							case 0x29:
							case 0x2A:
							case 0x2B:
							case 0x2C:
							case 0x2D:
							case 0x2E:
							case 0x2F:
							{
								--R[N];
								break;
							}
							case 0x30:
							{
								uint8_t data = 0;
								if (memory_read_func != nullptr && userdata != nullptr)
								{
									data = memory_read_func(R[P], userdata);
								}
								R[P] &= ~(0xFF);
								R[P] |= data;
								break;
							}
							case 0x31:
							{
								if (Q == 1)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x32:
							{
								if (D == 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x33:
							{
								if (DF == 1)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x34:
							{
								if (EF[0])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x35:
							{
								if (EF[1])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x36:
							{
								if (EF[2])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x37:
							{
								if (EF[3])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x38:
							{
								++R[P];
								break;
							}
							case 0x39:
							{
								if (Q == 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x3A:
							{
								if (D != 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x3B:
							{
								if (DF == 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x3C:
							{
								if (!EF[0])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x3D:
							{
								if (!EF[1])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x3E:
							{
								if (!EF[2])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x3F:
							{
								if (!EF[3])
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									R[P] &= ~(0xFF);
									R[P] |= data;
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0x40:
							case 0x41:
							case 0x42:
							case 0x43:
							case 0x44:
							case 0x45:
							case 0x46:
							case 0x47:
							case 0x48:
							case 0x49:
							case 0x4A:
							case 0x4B:
							case 0x4C:
							case 0x4D:
							case 0x4E:
							case 0x4F:
							{
								D = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[N], userdata) : 0;
								++R[N];
								break;
							}
							case 0x50:
							case 0x51:
							case 0x52:
							case 0x53:
							case 0x54:
							case 0x55:
							case 0x56:
							case 0x57:
							case 0x58:
							case 0x59:
							case 0x5A:
							case 0x5B:
							case 0x5C:
							case 0x5D:
							case 0x5E:
							case 0x5F:
							{
								if (memory_write_func != nullptr && userdata != nullptr)
								{
									memory_write_func(R[N], D, userdata);
								}
								break;
							}
							case 0x60:
							{
								++R[X];
								break;
							}
							case 0x61:
							case 0x62:
							case 0x63:
							case 0x64:
							case 0x65:
							case 0x66:
							case 0x67:
							{
								uint8_t data = 0;
								N0 = (N & 0x1);
								N1 = (N & 0x2);
								N2 = (N & 0x4);
								if (out_func != nullptr && userdata != nullptr)
								{
									if (memory_read_func != nullptr)
									{
										data = memory_read_func(R[X], userdata);
									}
									out_func(N, data, userdata);
								}
								++R[X];
								break;
							}
							case 0x69:
							case 0x6A:
							case 0x6B:
							case 0x6C:
							case 0x6D:
							case 0x6E:
							case 0x6F:
							{
								uint8_t data = 0;
								N0 = (N & 0x1);
								N1 = (N & 0x2);
								N2 = (N & 0x4);
								if (in_func != nullptr && userdata != nullptr)
								{
									data = in_func(N, userdata);
									if (memory_write_func != nullptr)
									{
										memory_write_func(R[X], data, userdata);
									}
								}
								D = data;
								break;
							}
							case 0x70:
							{
								uint8_t data = 0;
								if (memory_read_func != nullptr && userdata != nullptr)
								{
									data = memory_read_func(R[X], userdata);
								}
								++R[X];
								X = (data >> 4);
								P = (data & 0xF);
								// ++R[X];
								IE = 1;
								break;
							}
							case 0x71:
							{
								uint8_t data = 0;
								if (memory_read_func != nullptr && userdata != nullptr)
								{
									data = memory_read_func(R[X], userdata);
								}
								++R[X];
								X = (data >> 4);
								P = (data & 0xF);
								IE = 0;
								break;
							}
							case 0x72:
							{
								D = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0;
								++R[X];
								break;
							}
							case 0x73:
							{
								if (memory_write_func != nullptr && userdata != nullptr)
								{
									memory_write_func(R[X], D, userdata);
								}
								--R[X];
								break;
							}
							case 0x74:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0);
								uint8_t tmp = data + D + DF;
								DF = (tmp < data);
								D = tmp;
								break;
							}
							case 0x75:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0);
								uint8_t tmp = data - D - (~(DF) & 0x1);
								DF = (tmp < data);
								D = tmp;
								break;
							}
							case 0x76:
							{
								uint8_t tmp = (D & 0x1);
								D >>= 1;
								D |= (DF << 7);
								DF = tmp;
								break;
							}
							case 0x77:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0);
								uint8_t tmp = D - data - (~(DF) & 0x1);
								DF = (tmp < data);
								D = tmp;
								break;
							}
							case 0x78:
							{
								if (memory_write_func != nullptr && userdata != nullptr)
								{
									memory_write_func(R[X], T, userdata);
								}
								break;
							}
							case 0x79:
							{
								T = (X << 4) | P;
								if (memory_write_func != nullptr && userdata != nullptr)
								{
									memory_write_func(R[2], (X << 4) | P, userdata);
								}
								X = P;
								--R[2];
								break;
							}
							case 0x7A:
							{
								Q = 0;
								break;
							}
							case 0x7B:
							{
								Q = 1;
								break;
							}
							case 0x7C:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0);
								uint8_t tmp = data + D + DF;
								DF = (tmp < data);
								D = tmp;
								++R[P];
								break;
							}
							case 0x7D:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0);
								uint8_t tmp = data - D - (~(DF) & 0x01);
								DF = (tmp < data);
								D = tmp;
								++R[P];
								break;
							}
							case 0x7E:
							{
								uint8_t tmp = (D >> 7);
								D <<= 1;
								D |= DF;
								DF = tmp;
								break;
							}
							case 0x7F:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0);
								uint8_t tmp = D - data - (~(DF) & 0x01);
								DF = (tmp < data);
								D = tmp;
								++R[P];
								break;
							}
							case 0x80:
							case 0x81:
							case 0x82:
							case 0x83:
							case 0x84:
							case 0x85:
							case 0x86:
							case 0x87:
							case 0x88:
							case 0x89:
							case 0x8A:
							case 0x8B:
							case 0x8C:
							case 0x8D:
							case 0x8E:
							case 0x8F:
							{
								D = (R[N] & 0xFF);
								break;
							}
							case 0x90:
							case 0x91:
							case 0x92:
							case 0x93:
							case 0x94:
							case 0x95:
							case 0x96:
							case 0x97:
							case 0x98:
							case 0x99:
							case 0x9A:
							case 0x9B:
							case 0x9C:
							case 0x9D:
							case 0x9E:
							case 0x9F:
							{
								D = (R[N] >> 8);
								break;
							}
							case 0xA0:
							case 0xA1:
							case 0xA2:
							case 0xA3:
							case 0xA4:
							case 0xA5:
							case 0xA6:
							case 0xA7:
							case 0xA8:
							case 0xA9:
							case 0xAA:
							case 0xAB:
							case 0xAC:
							case 0xAD:
							case 0xAE:
							case 0xAF:
							{
								R[N] &= ~(0xFF);
								R[N] |= D;
								break;
							}
							case 0xB0:
							case 0xB1:
							case 0xB2:
							case 0xB3:
							case 0xB4:
							case 0xB5:
							case 0xB6:
							case 0xB7:
							case 0xB8:
							case 0xB9:
							case 0xBA:
							case 0xBB:
							case 0xBC:
							case 0xBD:
							case 0xBE:
							case 0xBF:
							{
								R[N] &= ~(0xFF00);
								R[N] |= (D << 8);
								break;
							}
							case 0xC0:
							{
								uint8_t data = 0;
								if (memory_read_func != nullptr && userdata != nullptr)
								{
									data = memory_read_func(R[P], userdata);
								}
								if (execute_cycles_left > 1)
								{
									B = data;
									++R[P];
								}
								else
								{
									R[P] = (B << 8) | data;
								}
								break;
							}
							case 0xC1:
							{
								if (Q == 1)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									if (execute_cycles_left > 1)
									{
										B = data;
										++R[P];
									}
									else
									{
										R[P] = (B << 8) | data;
									}
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0xC2:
							{
								if (D == 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									if (execute_cycles_left > 1)
									{
										B = data;
										++R[P];
									}
									else
									{
										R[P] = (B << 8) | data;
									}
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0xC3:
							{
								if (DF == 1)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									if (execute_cycles_left > 1)
									{
										B = data;
										++R[P];
									}
									else
									{
										R[P] = (B << 8) | data;
									}
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0xC4:
							{
								break;
							}
							case 0xC5:
							{
								if (Q == 0)
								{
									++R[P];
								}
								break;
							}
							case 0xC6:
							{
								if (D != 0)
								{
									++R[P];
								}
								break;
							}
							case 0xC7:
							{
								if (DF == 0)
								{
									++R[P];
								}
								break;
							}
							case 0xC8:
							{
								++R[P];
								break;
							}
							case 0xC9:
							{
								if (Q == 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									if (execute_cycles_left > 1)
									{
										B = data;
										++R[P];
									}
									else
									{
										R[P] = (B << 8) | data;
									}
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0xCA:
							{
								if (D != 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									if (execute_cycles_left > 1)
									{
										B = data;
									}
									else
									{
										R[P] = (B << 8) | data;
									}
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0xCB:
							{
								if (DF == 0)
								{
									uint8_t data = 0;
									if (memory_read_func != nullptr && userdata != nullptr)
									{
										data = memory_read_func(R[P], userdata);
									}
									if (execute_cycles_left > 1)
									{
										B = data;
										++R[P];
									}
									else
									{
										R[P] = (B << 8) | data;
									}
								}
								else
								{
									++R[P];
								}
								break;
							}
							case 0xCC:
							{
								if (IE == 1)
								{
									++R[P];
								}
								break;
							}
							case 0xCD:
							{
								if (Q == 1)
								{
									++R[P];
								}
								break;
							}
							case 0xCE:
							{
								if (D == 0)
								{
									++R[P];
								}
								break;
							}
							case 0xCF:
							{
								if (DF == 1)
								{
									++R[P];
								}
								break;
							}
							case 0xD0:
							case 0xD1:
							case 0xD2:
							case 0xD3:
							case 0xD4:
							case 0xD5:
							case 0xD6:
							case 0xD7:
							case 0xD8:
							case 0xD9:
							case 0xDA:
							case 0xDB:
							case 0xDC:
							case 0xDD:
							case 0xDE:
							case 0xDF:
							{
								P = N;
								break;
							}
							case 0xE0:
							case 0xE1:
							case 0xE2:
							case 0xE3:
							case 0xE4:
							case 0xE5:
							case 0xE6:
							case 0xE7:
							case 0xE8:
							case 0xE9:
							case 0xEA:
							case 0xEB:
							case 0xEC:
							case 0xED:
							case 0xEE:
							case 0xEF:
							{
								X = N;
								break;
							}
							case 0xF0:
							{
								D = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0;
								break;
							}
							case 0xF1:
							{
								D |= (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0;
								break;
							}
							case 0xF2:
							{
								D &= (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0;
								break;
							}
							case 0xF3:
							{
								D ^= (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0;
								break;
							}
							case 0xF4:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0);
								uint8_t tmp = data + D;
								DF = (tmp < data);
								D = tmp;
								break;
							}
							case 0xF5:
							{
								uint8_t data = ((memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0);
								uint8_t tmp = data - D;
								DF = (tmp < data);
								D = tmp;
								break;
							}
							case 0xF6:
							{
								DF = (D & 0x1);
								D >>= 1;
								break;
							}
							case 0xF7:
							{
								uint8_t data = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[X], userdata) : 0;
								uint8_t tmp = D - data;
								DF = (tmp < D);
								D = tmp;
								break;
							}
							case 0xF8:
							{
								D = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0;
								++R[P];
								break;
							}
							case 0xF9:
							{
								D |= (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0;
								++R[P];
								break;
							}
							case 0xFA:
							{
								D &= (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0;
								++R[P];
								break;
							}
							case 0xFB:
							{
								D ^= (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0;
								++R[P];
								break;
							}
							case 0xFC:
							{
								uint8_t data = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0;
								uint8_t tmp = data + D;
								DF = (tmp < data);
								D = tmp;
								++R[P];
								break;
							}
							case 0xFD:
							{
								uint8_t data = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0;
								uint8_t tmp = data - D;
								DF = (tmp < data);
								D = tmp;
								++R[P];
								break;
							}
							case 0xFE:
							{
								DF = (D >> 7);
								D <<= 1;
								break;
							}
							case 0xFF:
							{
								uint8_t data = (memory_read_func != nullptr && userdata != nullptr) ? memory_read_func(R[P], userdata) : 0;
								uint8_t tmp = D - data;
								DF = (tmp < D);
								D = tmp;
								++R[P];
								break;
							}
						}
						--execute_cycles_left;
						if (!execute_cycles_left)
						{
							if (dma_in_request)
							{
								dma_in_request = false;
								DMATransferData data = { DMAType::DMAIn, CurrentDMAInRequest.userdata, CurrentDMAInRequest.func };
								for (uint16_t i = 0; i < CurrentDMAInRequest.bytes_to_transfer; ++i)
								{
									DMATransferQueue.push_back(data);
								}
								CurrentDMAInRequest = { 0, nullptr, nullptr };
								CurrentCycleState = CycleState::DMA;
							}
							else if (dma_out_request)
							{
								dma_out_request = false;
								DMATransferData data = { DMAType::DMAOut, CurrentDMAOutRequest.userdata, CurrentDMAOutRequest.func };
								for (uint16_t i = 0; i < CurrentDMAOutRequest.bytes_to_transfer; ++i)
								{
									DMATransferQueue.push_back(data);
								}
								CurrentDMAOutRequest = { 0, nullptr, nullptr };
								CurrentCycleState = CycleState::DMA;
							}
							else if (interrupt_request)
							{
								interrupt_request = false;
								CurrentCycleState = IE ? CycleState::Interrupt : CycleState::Fetch;
							}
							else
							{
								CurrentCycleState = CycleState::Fetch;
							}
						}
					}
					else
					{
						if (dma_in_request)
						{
							idle = false;
							dma_in_request = false;
							DMATransferData data = { DMAType::DMAIn, CurrentDMAInRequest.userdata, CurrentDMAInRequest.func };
							for (uint16_t i = 0; i < CurrentDMAInRequest.bytes_to_transfer; ++i)
							{
								DMATransferQueue.push_back(data);
							}
							CurrentDMAInRequest = { 0, nullptr, nullptr };
							CurrentCycleState = CycleState::DMA;
						}
						else if (dma_out_request)
						{
							idle = false;
							dma_out_request = false;
							DMATransferData data = { DMAType::DMAOut, CurrentDMAOutRequest.userdata, CurrentDMAOutRequest.func };
							for (uint16_t i = 0; i < CurrentDMAOutRequest.bytes_to_transfer; ++i)
							{
								DMATransferQueue.push_back(data);
							}
							CurrentDMAInRequest = { 0, nullptr, nullptr };
							CurrentCycleState = CycleState::DMA;
						}
						else if (interrupt_request)
						{
							idle = false;
							interrupt_request = false;
							CurrentCycleState = IE ? CycleState::Interrupt : CycleState::Fetch;
						}
					}
					break;
				}
				case CycleState::DMA:
				{
					DMATransferData transfer_data = DMATransferQueue.front();
					DMATransferQueue.pop_front();
					uint8_t data = 0;
					if (transfer_data.type == DMAType::DMAOut)
					{
						if (memory_read_func != nullptr && userdata != nullptr)
						{
							data = memory_read_func(R[0], userdata);
						}
					}
					transfer_data.func(&data, transfer_data.userdata);
					if (transfer_data.type == DMAType::DMAIn)
					{
						if (memory_write_func != nullptr && userdata != nullptr)
						{
							memory_write_func(R[0], data, userdata);
						}
						D = data;
					}
					++R[0];
					if (DMATransferQueue.size() == 0)
					{
						if (dma_in_request)
						{
							dma_in_request = false;
							transfer_data = { DMAType::DMAIn, CurrentDMAInRequest.userdata, CurrentDMAInRequest.func };
							for (uint16_t i = 0; i < CurrentDMAInRequest.bytes_to_transfer; ++i)
							{
								DMATransferQueue.push_back(transfer_data);
							}
							CurrentDMAInRequest = { 0, nullptr, nullptr };
						}
						else if (dma_out_request)
						{
							dma_out_request = false;
							transfer_data = { DMAType::DMAOut, CurrentDMAOutRequest.userdata, CurrentDMAOutRequest.func };
							for (uint16_t i = 0; i < CurrentDMAOutRequest.bytes_to_transfer; ++i)
							{
								DMATransferQueue.push_back(transfer_data);
							}
							CurrentDMAOutRequest = { 0, nullptr, nullptr };
						}
						else if (interrupt_request)
						{
							interrupt_request = false;
							CurrentCycleState = IE ? CycleState::Interrupt : CycleState::Fetch;
						}
						else
						{
							CurrentCycleState = CycleState::Fetch;
						}
					}
					break;
				}
				case CycleState::Interrupt:
				{
					T = (X << 4) | P;
					X = 2;
					P = 1;
					IE = 0;
					CurrentCycleState = CycleState::Fetch;
					break;
				}
			}
		}
	}
}
