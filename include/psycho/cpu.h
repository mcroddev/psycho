// SPDX-License-Identifier: MIT
//
// Copyright 2024 Michael Rodriguez
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// @file cpu.h Provides public information about the CPU interpreter.

#pragma once

#include "bus.h"
#include "dbg_log.h"
#include "cpu_defs.h"
#include "types.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
struct psycho_cpu {
	u32 gpr[PSYCHO_CPU_GPR_REGS_NUM];
	u32 cp0_cpr[PSYCHO_CPU_CP0_CPR_REGS_NUM];

	/// @brief Geometry Transformation Engine (GTE, COP2) coprocessor
	/// registers.
	///
	/// @note The implementation makes extensive use of type-punning through
	/// unions to avoid not only explicit casting but for overall code
	/// clarity. While this is officially undefined behavior, it is
	/// permitted as a gcc/clang extension. Issues that are raised regarding
	/// this behavior will be closed as not a bug. Please refer to the gcc
	/// documentation excerpt at https://preview.tinyurl.com/typepun.
	struct {
		/// @brief Data registers.
		union {
			struct {
				s16 V[3][3];   // 0-5
				u8 RGBC[4];    // 6
				u16 OTZ;       // 7
				s16 IR[4];     // 8-11
				s16 SXY[3][2]; // 12-14
				u16 SZ[4];     // 16-19
				u8 RGB[3][4];  // 20-22
				s32 MAC[4];    // 24-27
				s32 LZCS;      // 30
			};
			u32 regs[PSYCHO_CPU_CP2_CPR_REGS_NUM];
		} cpr;

		/// @brief Control registers.
		union {
			struct {
				s16 RT[3][3];  // 0-4
				s32 TR[3];     // 5-7
				s16 LCM[3][3]; // 8-12
				s32 BK[3];     // 13-15
				s16 LLM[3][3]; // 16-20
				s32 FC[3];     // 21-23
				s32 OFX;       // 24
				s32 OFY;       // 25
				u16 H;         // 26
				s32 DQA;       // 27
				s32 DQB;       // 28
				s32 ZSF3;      // 29
				s32 ZSF4;      // 30
				u32 FLAG;      // 31
			};
			u32 regs[PSYCHO_CPU_CP2_CCR_REGS_NUM];
		} ccr;
	} cp2;

	struct {
		uint dst;
		u32 val;
	} lds_pend, lds_next;

	u32 instr;
	u32 pc;
	u32 npc;

	u32 hi;
	u32 lo;

	u16 exc_halt;

	struct psycho_bus *bus;
	struct psycho_dbg_log *log;
};
#pragma GCC diagnostic pop
